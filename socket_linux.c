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

#include "debug.h"
#include "mm.h"

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

static struct FluffMMGroup * mm_group = NULL;
static struct FluffMMType * socket_size = NULL;
static struct FluffMMType * addr_size = NULL;
static struct FluffMMType * set_size = NULL;
static struct FluffMMType * iter_size = NULL;

static void setup_mm_types(){
    mm_group = fluff_mm_new_g();
    socket_size = fluff_mm_new(sizeof(struct FluffSocket), mm_group);
    addr_size = fluff_mm_new(sizeof(struct FluffSocketAddr), mm_group);
    set_size = fluff_mm_new(sizeof(struct FluffSocketSet), mm_group);
    iter_size = fluff_mm_new(sizeof(struct FluffSocketIter), mm_group);
}

#define ENSURE_MM if (!mm_group) setup_mm_types();

static int set_blocking(int sock, int blocking){
	int flags;

	flags = fcntl(sock, F_GETFL, 0);
	if (flags < 0){
		return flags;
	} else if (blocking){
		flags &= ~O_NONBLOCK;
	} else {
		flags |= O_NONBLOCK;
	}
	return fcntl(sock, F_SETFL, flags);
}

static struct FluffSocket * socket_new(int sock){
	struct FluffSocket * self;

	ENSURE_MM;

	if ((self = fluff_mm_alloc(socket_size))){
		self->sock = sock;
		self->next = NULL;
	}
	return self;
}

static int addr_getaddrinfo(
		struct FluffSocketAddr * addr,
		struct addrinfo * hints,
		struct addrinfo ** result){
	char service[6];
	unsigned int port;

	port = addr->port;
	snprintf(service, 6, "%u", port);
	return getaddrinfo(addr->address, service, hints, result);
}

static struct FluffSocketAddr * addr_fromsockaddr(
		struct sockaddr * src){
	struct FluffSocketAddr * self;

	ENSURE_MM;

	if ((self = fluff_mm_alloc(addr_size))){
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
				return NULL;
		}
	}

	return self;
}

struct FluffSocketAddr * fluff_socket_addr(char * host, uint16_t port){
	struct FluffSocketAddr * self;

	ENSURE_MM;

	if ((self = fluff_mm_alloc(addr_size))){
		self->port = port;
		strncpy(self->address, host, ADDRSTRLEN);
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

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_PASSIVE;

	if (addr_getaddrinfo(addr, &hints, &result)){
		return NULL;
	}

	for (res = result; res; res = res->ai_next){
		sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sock < 0){
			continue;
		}
		set_blocking(sock, blocking);
		if (reuse_addr){
			setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true_val, sizeof(int));
		}
		if (!bind(sock, res->ai_addr, res->ai_addrlen) && !listen(sock, 5)){
			break;
		} else {
			close(sock);
			sock = -1;
		}
	}
	freeaddrinfo(result);
	if (sock < 0){
		return NULL;
	} else {
		return socket_new(sock);
	}
}

struct FluffSocket * fluff_socket_connect(
		struct FluffSocketAddr * addr, int blocking){
	struct addrinfo hints;
	struct addrinfo * result, * res;
	int sock = -1;
	struct FluffSocket * self = NULL, * nself;

	ENSURE_MM;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_PASSIVE;

	if (addr_getaddrinfo(addr, &hints, &result)){
		return NULL;
	}

	for (res = result; res; res = res->ai_next){
		sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sock < 0){
			continue;
		}
		set_blocking(sock, blocking);
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
	return self;
}

int fluff_socket_connected(struct FluffSocket * self){
	int epfd;
	struct FluffSocket * node, * node2;
	struct epoll_event event;
	int res = 1, value;
	socklen_t optlen = sizeof(int);

	epfd = epoll_create(EPOLL_CREATE_SIZE);
	if (epfd < 0){
		LOGD("epoll_create failed");
		return -1;
	}
	node = self;
	while (node){
		event.events = EPOLLOUT;
		event.data.ptr = node;
		if (epoll_ctl(epfd, EPOLL_CTL_ADD, node->sock, &event)){
			LOGD("epoll_ctl failed");
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
				LOGD("getsockopt failed");
				res = -1;
				break;
			}
			if (value){
				if (self == node){
					if (!node->next){
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
			}
			fluff_socket_close(node);
		} else {
			LOGD("epoll_wait failed");
		}
	}
	close(epfd);
	return res;
}

struct FluffSocket * fluff_socket_accept(struct FluffSocket * self){
	struct FluffSocket * conn = NULL;
	int sock;
	struct sockaddr_storage addr;
	socklen_t addr_len = sizeof(struct sockaddr_storage);

	sock = accept(self->sock, (struct sockaddr *)&addr, &addr_len);
	if (sock >= 0){
		conn = socket_new(sock);
		conn->addr = addr_fromsockaddr((struct sockaddr *)&addr);
	}
	return conn;
}

ssize_t fluff_socket_recv(struct FluffSocket * self, void * buf, size_t size){
	ssize_t recvd;

	recvd = recv(self->sock, buf, size, 0);
	if (recvd < 0){
		if (errno == EAGAIN || errno == EWOULDBLOCK){
			recvd = 0;
		} else {
			recvd = -1;
		}
	}
	return recvd;
}

ssize_t fluff_socket_send(struct FluffSocket * self, void * buf, size_t size){
	ssize_t sent;

	sent = send(self->sock, buf, size, 0);
	if (sent < 0){
		if (errno == EAGAIN || errno == EWOULDBLOCK){
			sent = 0;
		} else {
			sent = -1;
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
		fluff_mm_free(to_del);
	}
}


struct FluffSocketSet * fluff_socket_set_new(){
	struct FluffSocketSet * self;
	int epfd;

	ENSURE_MM;

	LOGD("allocating socketset");
	if ((self = fluff_mm_alloc(set_size))){
		LOGD("socketset allocated");
		LOGD("initializing epoll");
		self->epfd = epfd = epoll_create(EPOLL_CREATE_SIZE);
		if (epfd < 0){
			fluff_mm_free(self);
			self = NULL;
		} else {
			LOGD("epoll initialized");
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

	event.data.ptr = sock;
	event.events = EPOLLRDHUP;
	if (do_read){
		event.events |= EPOLLIN;
	}
	if (do_write){
		event.events |= EPOLLOUT;
	}
	return epoll_ctl(self->epfd, EPOLL_CTL_ADD, sock->sock, &event);
}

int fluff_socket_set_mod(
		struct FluffSocketSet * self,
		struct FluffSocket * sock,
		int do_read,
		int do_write){
	struct epoll_event event;

	event.data.ptr = sock;
	event.events = EPOLLRDHUP;
	if (do_read){
		event.events |= EPOLLIN;
	}
	if (do_write){
		event.events |= EPOLLOUT;
	}
	return epoll_ctl(self->epfd, EPOLL_CTL_MOD, sock->sock, &event);
}

int fluff_socket_set_del(
		struct FluffSocketSet * self, struct FluffSocket * sock){
	struct epoll_event event;

	return epoll_ctl(self->epfd, EPOLL_CTL_DEL, sock->sock, &event);
}

void fluff_socket_set_free(struct FluffSocketSet * self){
	close(self->epfd);
	fluff_mm_free(self);
}

static int populate_iter(struct FluffSocketIter * iter, int timeout){
	int count;

	iter->count = count = epoll_wait(
			iter->epfd, iter->events, N_EVENTS, timeout);
	if (count < 0){
		return -1;
	}
	iter->index = 0;

	return 0;
}

struct FluffSocketIter * fluff_socket_select(
		struct FluffSocketSet * self, double timeout){
	struct FluffSocketIter * iter;

	if ((iter = fluff_mm_alloc(iter_size))){
		iter->epfd = self->epfd;
		if (populate_iter(iter, (int)(timeout * 1000))){
			fluff_mm_free(iter);
			iter = NULL;
		}
	}
	return iter;
}

int fluff_socket_iter_next(
		struct FluffSocketIter * self,
		struct FluffSocket ** dest,
		int * dest_read, int * dest_write, int * dest_error){
	static int error_cond = (EPOLLRDHUP | EPOLLERR | EPOLLHUP);

	while (self->index == self->count){
		if (self->count == N_EVENTS){
			if (populate_iter(self, 0)){
				return -1;
			}
		} else {
			*dest = NULL;
			*dest_read = 0;
			*dest_write = 0;
			*dest_error = 0;
			return 0;
		}
	}
	*dest = self->events[self->index].data.ptr;
	*dest_read = self->events[self->index].events & EPOLLIN;
	*dest_write = self->events[self->index].events & EPOLLOUT;
	*dest_error = self->events[self->index].events & error_cond;
	self->index += 1;
	return 0;
}

void fluff_socket_iter_free(struct FluffSocketIter * self){
	fluff_mm_free(self);
}

void fluff_socket_use_mm(int use){
	ENSURE_MM;
	fluff_mm_manage_g(mm_group, use);
}
