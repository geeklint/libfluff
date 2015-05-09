/*
	Copyright 2014 Sky Leonard
	This file is part of libfluff.

    libfluff is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libfluff is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libfluff.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "socket.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "mm.h"
#include "traceback.h"

#define EPOLL_CREATE_SIZE 5
#define N_EVENTS 16

/* Max length of hostname. Assume longer than any ip address */
#define ADDRSTRLEN 256

struct FluffSocket {
	int sock;
	struct FluffSocketAddr * addr;
	union FluffData userdata;
	struct FluffSocket * next;
};

struct FluffSocketAddr {
	char address[ADDRSTRLEN];
	uint16_t port;
};

struct FluffSocketSet {
	int epfd;
};

struct FluffSocketIter {
	int epfd;
	struct epoll_event events[N_EVENTS];
	int index;
	int count;
};

/*
 * Memory manager
 */

static int mm_need_setup = 1;
static const struct FluffMM * MM = NULL;

static union FluffData socket_size;
static union FluffData addr_size;
static union FluffData set_size;
static union FluffData iter_size;

static void setup_mm(){
	if (MM == NULL){
		MM = fluff_mm_default;
	}
    socket_size = MM->f_type_new(sizeof(struct FluffSocket));
    addr_size = MM->f_type_new(sizeof(struct FluffSocketAddr));
    set_size = MM->f_type_new(sizeof(struct FluffSocketSet));
    iter_size = MM->f_type_new(sizeof(struct FluffSocketIter));
    mm_need_setup = 0;
}

#define ENSURE_MM if (mm_need_setup) setup_mm();

void fluff_socket_setmm(const struct FluffMM * mm){
	if (!mm_need_setup){
		MM->f_type_free(socket_size);
		MM->f_type_free(addr_size);
		MM->f_type_free(set_size);
		MM->f_type_free(iter_size);
		mm_need_setup = 1;
	}
	MM = mm;
	setup_mm();
}

static int set_blocking(int sock, int blocking){
	int flags;

	if (!fluff_traceback_grab("set_blocking")){
		flags = fcntl(sock, F_GETFL, 0);
		if (flags < 0){
			fluff_traceback_set("failed to get flags");
			return flags;
		} else if (blocking){
			flags &= ~O_NONBLOCK;
		} else {
			flags |= O_NONBLOCK;
		}
		if (fcntl(sock, F_SETFL, flags) < 0){
			fluff_traceback_set("failed to set flags");
			return -1;
		} else {
			fluff_traceback_drop();
			return 0;
		}
	} else {
		return -1;
	}
}

static struct FluffSocket * socket_new(int sock){
	struct FluffSocket * self = NULL;

	ENSURE_MM;

	if (!fluff_traceback_grab("socket_new")){
		if ((self = MM->f_alloc(socket_size))){
			self->sock = sock;
			self->next = NULL;
			fluff_traceback_drop();
		} else {
			fluff_traceback_set("failed to allocate object");
		}
	}
	return self;
}

static int addr_getaddrinfo(
		struct FluffSocketAddr * addr,
		struct addrinfo * hints,
		struct addrinfo ** result){
	char service[6];
	unsigned int port;

	if (!fluff_traceback_grab("addr_getaddrinfo")){
		port = addr->port;
		snprintf(service, 6, "%u", port);
		if (getaddrinfo(addr->address, service, hints, result)){
			fluff_traceback_set("getaddrinfo failed");
			return -1;
		} else {
			fluff_traceback_drop();
			return 0;
		}
	} else {
		return -1;
	}
}

static struct FluffSocketAddr * addr_fromsockaddr(
		struct sockaddr * src){
	struct FluffSocketAddr * self = NULL;

	ENSURE_MM;

	if (!fluff_traceback_grab("addr_fromsockaddr")){
		if ((self = MM->f_alloc(addr_size))){
			switch (src->sa_family){
				case AF_INET:
					self->port = ntohs(((struct sockaddr_in *)src)->sin_port);
					inet_ntop(
							AF_INET,
							&((((struct sockaddr_in *)src)->sin_addr)),
							self->address,
							ADDRSTRLEN);
					break;

#ifdef AF_INET6
				case AF_INET6:
					self->port = ntohs(((struct sockaddr_in6 *)src)->sin6_port);
					inet_ntop(
							AF_INET6,
							&((((struct sockaddr_in6 *)src)->sin6_addr)),
							self->address,
							ADDRSTRLEN);
					break;
#endif
				default:
					fluff_traceback_set("unknown socket family");
					return NULL;
			}
		}
		fluff_traceback_drop();
	}
	return self;
}

struct FluffSocketAddr * fluff_socket_addr(char * host, uint16_t port){
	struct FluffSocketAddr * self = NULL;

	ENSURE_MM;

	if (!fluff_traceback_grab("fluff_socket_addr")){
		if ((self = MM->f_alloc(addr_size))){
			self->port = port;
			strncpy(self->address, host, ADDRSTRLEN);
			fluff_traceback_drop();
		} else {
			fluff_traceback_set("failed to allocate object");
		}
	}
	return self;
}

void fluff_socket_addr_value(
		struct FluffSocketAddr * self, char ** host, uint16_t * port){
	*host = self->address;
	*port = self->port;
}

struct FluffSocket * fluff_socket_bind(
		struct FluffSocketAddr * addr, int blocking, int reuse_addr){
	struct addrinfo hints;
	struct addrinfo * result, * res;
	int true_val = 1;
	int sock = -1;

	ENSURE_MM;

	if (!fluff_traceback_grab("fluff_socket_bind")){
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = 0;
		hints.ai_flags = AI_PASSIVE;

		if (addr_getaddrinfo(addr, &hints, &result)){
			fluff_traceback_set("getaddrinfo failed");
			return NULL;
		}

		for (res = result; res; res = res->ai_next){
			sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
			if (sock < 0){
				continue;
			}
			fluff_traceback_pause();
			set_blocking(sock, blocking);
			fluff_traceback_resume();
			if (reuse_addr){
				setsockopt(
					sock, SOL_SOCKET, SO_REUSEADDR, &true_val, sizeof(int));
			}
			if (!bind(sock, res->ai_addr, res->ai_addrlen)
					&& !listen(sock, 5)){
				break;
			} else {
				close(sock);
				sock = -1;
			}
		}
		freeaddrinfo(result);
		if (sock < 0){
			fluff_traceback_set("no socket could be bound");
			return NULL;
		} else {
			fluff_traceback_drop();
			return socket_new(sock);
		}
	} else {
		return NULL;
	}
}

struct FluffSocket * fluff_socket_connect(
		struct FluffSocketAddr * addr, int blocking){
	struct addrinfo hints;
	struct addrinfo * result, * res;
	int sock = -1;
	struct FluffSocket * self = NULL, * nself;

	ENSURE_MM;

	if (!fluff_traceback_grab("fluff_socket_connect")){
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = 0;
		hints.ai_flags = AI_PASSIVE;

		if (addr_getaddrinfo(addr, &hints, &result)){
			fluff_traceback_set("getaddrinfo failed");
			return NULL;
		}

		for (res = result; res; res = res->ai_next){
			sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
			if (sock < 0){
				continue;
			}
			fluff_traceback_pause();
			set_blocking(sock, blocking);
			fluff_traceback_resume();
			if (!connect(sock, res->ai_addr, res->ai_addrlen)){
				fluff_socket_close(self);
				self = socket_new(sock);
				break;
			}
			if (errno == EINPROGRESS){
				nself = socket_new(sock);
				nself->next = self;
				self = nself;
			} else {
				close(sock);
				sock = -1;
			}
		}
		freeaddrinfo(result);
		if (self){
			fluff_traceback_drop();
		} else {
			fluff_traceback_set("no socket could be connected");
		}
	}
	return self;
}

int fluff_socket_connected(struct FluffSocket * self){
	int epfd;
	struct FluffSocket * node, * node2;
	struct epoll_event event;
	int res = 1, value;
	socklen_t optlen = sizeof(int);

	if (fluff_traceback_grab("fluff_socket_connected")){
		return -1;
	} else {
		epfd = epoll_create(EPOLL_CREATE_SIZE);
		if (epfd < 0){
			fluff_traceback_set("epoll_create failed");
			return -1;
		}
		node = self;
		while (node){
			event.events = EPOLLOUT;
			event.data.ptr = node;
			if (epoll_ctl(epfd, EPOLL_CTL_ADD, node->sock, &event)){
				fluff_traceback_set("epoll_ctl failed");
				close(epfd);
				return -1;
			}
			node = node->next;
		}
		while (res > 0){
			res = epoll_wait(epfd, &event, 1, 0);
			if (res > 0 && (event.events & EPOLLOUT)){
				node = event.data.ptr;
				if (getsockopt(
						node->sock, SOL_SOCKET, SO_ERROR, &value, &optlen)){
					fluff_traceback_set("getsockopt failed");
					res = -1;
					break;
				}
				if (value){
					if (self == node){
						if (!node->next){
							fluff_traceback_set("no remaining attempts");
							res = -1;
							break;
						} else {
							node = node->next;
							value = node->sock;
							node->sock = self->sock;
							self->sock = value;
							self->next = node->next;
							node->next = NULL;
						}
					} else {
						node2 = self;
						while (node2->next){
							if (node2->next == node){
								node2->next = node->next;
								node->next = NULL;
								break;
							}
						}
					}
				} else {
					if (self != node){
						value = node->sock;
						node->sock = self->sock;
						self->sock = value;
					}
					node = self->next;
					fluff_traceback_drop();
				}
				fluff_socket_close(node);
			} else {
				fluff_traceback_set("epoll_wait failed");
			}
		}
		close(epfd);
		return res;
	}
}

struct FluffSocket * fluff_socket_accept(struct FluffSocket * self){
	struct FluffSocket * conn = NULL;
	int sock;
	struct sockaddr_storage addr;
	socklen_t addr_len = sizeof(struct sockaddr_storage);

	if (!fluff_traceback_grab("fluff_socket_accept")){
		sock = accept(self->sock, (struct sockaddr *)&addr, &addr_len);
		if (sock >= 0){
			conn = socket_new(sock);
			if (!conn){
				fluff_traceback_set("socket_new failed");
			} else {
				conn->addr = addr_fromsockaddr((struct sockaddr *)&addr);
				if (!conn->addr){
					fluff_socket_close(conn);
					conn = NULL;
					fluff_traceback_set("addr_fromsockaddr failed");
				}
			}
		} else {
			fluff_traceback_set("accept failed");
		}
	}
	return conn;
}

long int fluff_socket_recv(struct FluffSocket * self, void * buf, size_t size){
	long int recvd = -1;

	if (!fluff_traceback_grab("fluff_socket_recv")){
		recvd = recv(self->sock, buf, size, 0);
		if (recvd < 0){
			if (errno == EAGAIN || errno == EWOULDBLOCK){
				recvd = 0;
				fluff_traceback_drop();
			} else {
				recvd = -1;
				fluff_traceback_set("recv failed");
			}
		} else {
			fluff_traceback_drop();
		}
	}
	return recvd;
}

ssize_t fluff_socket_send(struct FluffSocket * self, void * buf, size_t size){
	ssize_t sent = -1;

	if (!fluff_traceback_grab("fluff_socket_send")){
		sent = send(self->sock, buf, size, 0);
		if (sent < 0){
			if (errno == EAGAIN || errno == EWOULDBLOCK){
				sent = 0;
				fluff_traceback_drop();
			} else {
				sent = -1;
				fluff_traceback_set("send failed");
			}
		} else {
			fluff_traceback_drop();
		}
	}
	return sent;
}

struct FluffSocketAddr * fluff_socket_addr_get(struct FluffSocket * self){
	return self->addr;
}

void fluff_socket_ud_set(struct FluffSocket * self, union FluffData ud){
	self->userdata = ud;
}

union FluffData fluff_socket_ud_get(struct FluffSocket * self){
	return self->userdata;
}

void fluff_socket_close(struct FluffSocket * self){
	struct FluffSocket * to_del;

	while (self){
		to_del = self;
		self = self->next;
		close(to_del->sock);
		MM->f_free(to_del);
	}
}


struct FluffSocketSet * fluff_socket_set_new(){
	struct FluffSocketSet * self = NULL;
	int epfd;

	ENSURE_MM;

	if (!fluff_traceback_grab("fluff_socket_set_new")){
		if ((self = MM->f_alloc(set_size))){
			self->epfd = epfd = epoll_create(EPOLL_CREATE_SIZE);
			if (epfd < 0){
				MM->f_free(self);
				fluff_traceback_set("epoll_create failed");
				self = NULL;
			} else {
				fluff_traceback_drop();
			}
		} else {
			fluff_traceback_set("failed to allocate object");
		}
	}
	return self;
}

int fluff_socket_set_add(
		struct FluffSocketSet * self,
		struct FluffSocket * sock,
		int do_read,
		int do_write){
	struct epoll_event event;
	int res = -1;

	if (!fluff_traceback_grab("fluff_socket_set_add")){
		event.data.ptr = sock;
		event.events = EPOLLRDHUP;
		if (do_read){
			event.events |= EPOLLIN;
		}
		if (do_write){
			event.events |= EPOLLOUT;
		}
		res = epoll_ctl(self->epfd, EPOLL_CTL_ADD, sock->sock, &event);
		if (res){
			fluff_traceback_set("epoll_ctl failed");
		} else {
			fluff_traceback_drop();
		}
	}
	return res;
}

int fluff_socket_set_mod(
		struct FluffSocketSet * self,
		struct FluffSocket * sock,
		int do_read,
		int do_write){
	struct epoll_event event;
	int res = -1;

	if (!fluff_traceback_grab("fluff_socket_set_mod")){
		event.data.ptr = sock;
		event.events = EPOLLRDHUP;
		if (do_read){
			event.events |= EPOLLIN;
		}
		if (do_write){
			event.events |= EPOLLOUT;
		}
		res = epoll_ctl(self->epfd, EPOLL_CTL_MOD, sock->sock, &event);
		if (res){
			fluff_traceback_set("epoll_ctl failed");
		} else {
			fluff_traceback_drop();
		}
	}
	return res;
}

int fluff_socket_set_del(
		struct FluffSocketSet * self, struct FluffSocket * sock){
	struct epoll_event event;
	int res = -1;

	if (!fluff_traceback_grab("fluff_socket_set_del")){
		res = epoll_ctl(self->epfd, EPOLL_CTL_DEL, sock->sock, &event);
		if (res){
			fluff_traceback_set("epoll_ctl failed");
		} else {
			fluff_traceback_drop();
		}
	}
	return res;
}

void fluff_socket_set_free(struct FluffSocketSet * self){
	close(self->epfd);
	MM->f_free(self);
}

static int populate_iter(struct FluffSocketIter * iter, int timeout){
	int count;

	if (!fluff_traceback_grab("populate_iter")){
		iter->index = 0;
		iter->count = count = epoll_wait(
				iter->epfd, iter->events, N_EVENTS, timeout);
		if (count < 0){
			iter->count = 0;
			fluff_traceback_set("epoll_wait failed");
			return -1;
		} else {
			fluff_traceback_drop();
			return 0;
		}
	}
	return -1;
}

struct FluffSocketIter * fluff_socket_select(
		struct FluffSocketSet * self, double timeout){
	struct FluffSocketIter * iter = NULL;

	if (!fluff_traceback_grab("fluff_socket_select")){
		if ((iter = MM->f_alloc(iter_size))){
			iter->epfd = self->epfd;
			if (populate_iter(iter, (int)(timeout * 1000))){
				MM->f_free(iter);
				iter = NULL;
				fluff_traceback_set("populate_iter failed");
			} else {
				fluff_traceback_drop();
			}
		} else {
			fluff_traceback_set("allocating iter failed");
		}
	}
	return iter;
}

int fluff_socket_iter_next(
		struct FluffSocketIter * self,
		struct FluffSocket ** dest,
		int * dest_read, int * dest_write, int * dest_error){
	static int error_cond = (EPOLLRDHUP | EPOLLERR | EPOLLHUP);

	if (fluff_traceback_grab("fluff_socket_iter_next")){
		return -1;
	} else {
		if (self->index == self->count){
			if (self->count == N_EVENTS){
				if (populate_iter(self, 0)){
					fluff_traceback_set("populate_iter failed");
					return -1;
				}
			} else {
				*dest = NULL;
				*dest_read = 0;
				*dest_write = 0;
				*dest_error = 0;
				fluff_traceback_drop();
				return 0;
			}
		}
		*dest = self->events[self->index].data.ptr;
		*dest_read = self->events[self->index].events & EPOLLIN;
		*dest_write = self->events[self->index].events & EPOLLOUT;
		*dest_error = self->events[self->index].events & error_cond;
		self->index += 1;
		fluff_traceback_drop();
		return 0;
	}
}

void fluff_socket_iter_free(struct FluffSocketIter * self){
	MM->f_free(self);
}
