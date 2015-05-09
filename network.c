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

#include "network.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "mm.h"
#include "set.h"
#include "traceback.h"

struct FluffNetwork {
	enum FluffNetworkSendOpt send_opt;
	FluffFreeFunction send_freer;
	struct FluffSocketSet * sockets;
	struct FluffSetElement * connecting;
	struct FluffSetElement * handles;
};

enum HandleType {
	HandleTypeListen,
	HandleTypeConnect,
	HandleTypeRemote,
};

struct BufferNode {
	void * data;
	size_t offset;
	size_t size;
	struct BufferNode * next;
};

struct FluffNetworkHandle {
	struct FluffSocket * sock;
	enum HandleType type;
	struct FluffNetwork * network;
	union FluffData userdata;
	FluffFreeFunction ud_freer;
	struct BufferNode * buffer_head;
	struct BufferNode * buffer_tail;
	FluffNetworkCallback on_recv;
	FluffNetworkCallback on_error;
	struct FluffSetElementElement * element;
};

/*
 * Memory manager
 */

static int mm_need_setup = 1;
static const struct FluffMM * MM = NULL;

static union FluffData network_size;
static union FluffData bufnode_size;
static union FluffData handle_size;

static void setup_mm(){
	if (MM == NULL){
		MM = fluff_mm_default;
	}
    network_size = MM->f_type_new(sizeof(struct FluffNetwork));
    bufnode_size = MM->f_type_new(sizeof(struct BufferNode));
    handle_size = MM->f_type_new(sizeof(struct FluffNetworkHandle));
}

#define ENSURE_MM if (mm_need_setup) setup_mm();

void fluff_network_setmm(const struct FluffMM * mm){
	if (!mm_need_setup){
		MM->f_type_free(network_size);
		MM->f_type_free(bufnode_size);
		MM->f_type_free(handle_size);
		mm_need_setup = 1;
	}
	MM = mm;
	setup_mm();
}

/*
 * Internal (static) methods and functions
 */

static struct FluffNetworkHandle * handle_new(
		struct FluffSocket * sock,
		enum HandleType type,
		struct FluffNetwork * network){
	struct FluffNetworkHandle * self = NULL;
	if (!fluff_traceback_grab("handle_new")){
		if ((self = MM->f_alloc(handle_size))){
			self->sock = sock;
			self->type = type;
			self->network = network;
			self->userdata = fluff_data_zero;
			self->buffer_head = NULL;
			self->buffer_tail = NULL;
			self->on_recv = NULL;
			self->on_error = NULL;
			fluff_traceback_drop();
		} else {
			fluff_traceback_set("failed to allocate handle");
		}
	}
	return self;
}

static void buffer_free(
		struct FluffNetwork * network, struct BufferNode * buffer){
	struct BufferNode * next;

	while (buffer){
		next = buffer->next;
		switch (network->send_opt) {
			case FluffNetworkSendOptCopy:
				free(buffer->data);
				break;
			case FluffNetworkSendOptFree:
				network->send_freer(buffer->data);
				break;
			default:
				break;
		}
		MM->f_free(buffer);
		buffer = next;
	}
}

static void handle_free(struct FluffNetworkHandle * self){
	fluff_socket_close(self->sock);
	buffer_free(self->network, self->buffer_head);
	if (self->ud_freer)
		self->ud_freer(self->userdata.d_ptr);
	fluff_set_element_remove(self->network->connecting, self->element);
	fluff_set_element_remove(self->network->handles, self->element);
	fluff_set_element_element_free(self->element);
	MM->f_free(self);
}

static int check_connecting(struct FluffNetwork * self){
	struct FluffNetworkHandle * handle;
	union FluffData handle_as_data;
	struct FluffSetElementElement * element;
	struct FluffSetElementIter * iter;
	int res;

	if (fluff_traceback_grab("check_connecting")){
		return -1;
	} else {
		iter = fluff_set_element_iter(self->connecting);
		if (!iter){
			fluff_traceback_set("creating iter failed");
			return -1;
		}
		while ((element = fluff_set_element_iter_next(iter))){
			handle_as_data = fluff_set_element_element_data_get(element);
			handle = handle_as_data.d_ptr;
			res = fluff_socket_connected(handle->sock);
			if (res < 0){
				fluff_traceback_set("check for connection failed");
				res = -1;
				break;
			} else if (res > 0){
				fluff_set_element_iter_removelast(iter);
				fluff_set_element_element_free(element);
				if (!(handle->element = element = fluff_set_element_element_new(
						self->handles))){
					fluff_traceback_set("creating new element failed");
					res = -1;
					break;
				}
				fluff_set_element_element_data_set(element, handle_as_data);
				handle->type = HandleTypeRemote;
				if (fluff_socket_set_add(self->sockets, handle->sock, 1, 1)){
					fluff_traceback_set("adding socket to set failed");
					handle_free(handle);
					res = -1;
					break;
				}
				fluff_set_element_add(self->handles, element);
			}
			res = 0;
		}
		if (!res){
			fluff_traceback_drop();
		}
		fluff_set_element_iter_free(iter);
		return res;
	}
}

/*
 * Attempt to send outgoing data to the client
 */
static int attempt_send(struct FluffNetworkHandle * handle){
    ssize_t res;
    struct BufferNode * node;

    if (!fluff_traceback_grab("attempt_send")){
		while ((node = handle->buffer_head)){
			res = fluff_socket_send(
				handle->sock,
				node->data + node->offset,
				node->size - node->offset);
			if (res < 0){
				fluff_traceback_set("send failed");
				return -1;
			} else if (res + node->offset >= node->size){
				handle->buffer_head = node->next;
				node->next = NULL;
				buffer_free(handle->network, node);
			} else {
				node->offset += res;
				break;
			}
		}
    }
    fluff_traceback_drop();
    return 0;
}

/*
 * Exported Functions
 */

struct FluffNetwork * fluff_network_new(
		enum FluffNetworkSendOpt send_opt, ...){
	struct FluffNetwork * self = NULL;
	va_list args;

	ENSURE_MM;

	if (!fluff_traceback_grab("fluff_network_new")){
		if ((self = MM->f_alloc(network_size))){
			if (!(self->sockets = fluff_socket_set_new())){
				fluff_traceback_set("failed to allocate set");
				MM->f_free(self);
				return NULL;
			}
			self->send_opt = send_opt;
			if (send_opt == FluffNetworkSendOptFree){
				va_start(args, send_opt);
				self->send_freer = va_arg(args, FluffFreeFunction);
				va_end(args);
			}
			self->connecting = NULL;
			self->handles = NULL;
			fluff_traceback_drop();
		} else {
			fluff_traceback_set("failed to allocate object");
		}
	}
	return self;
}

int fluff_network_bind(
		struct FluffNetwork * self,
		struct FluffSocketAddr * addr,
		FluffNetworkCallback cb){
	struct FluffSocket * sock;
	struct FluffNetworkHandle * handle;
	union FluffData handle_as_data;

	if (fluff_traceback_grab("fluff_network_bind")){
		return -1;
	} else {
		sock = fluff_socket_bind(addr, 0, 1);
		if (!sock){
			fluff_traceback_set("bind failed");
			return -1;
		}
		handle = handle_new(sock, HandleTypeListen, self);
		if (!handle){
			fluff_traceback_set("allocating handle failed");
			fluff_socket_close(sock);
			return -1;
		}
		handle->on_recv = cb;
		handle_as_data.d_ptr = handle;
		fluff_socket_ud_set(sock, handle_as_data);
		if (fluff_socket_set_add(self->sockets, sock, 1, 0)){
			fluff_traceback_set("adding socket to set failed");
			handle_free(handle);
			return -1;
		}
		if (!(handle->element = fluff_set_element_element_new(
				self->handles))){
	 		fluff_traceback_set("allocating set element failed");
			// Don't know what to do if cleanup fails
			fluff_traceback_pause();
			fluff_socket_set_del(self->sockets, sock);
			fluff_traceback_resume();
			handle_free(handle);
			return -1;
		}
		fluff_set_element_element_data_set(handle->element, handle_as_data);
		fluff_set_element_add(self->connecting, handle->element);
		fluff_traceback_drop();
		return 0;
	}
}

int fluff_network_connect(
		struct FluffNetwork * self,
		struct FluffSocketAddr * addr,
		FluffNetworkCallback cb){
	struct FluffSocket * sock;
	struct FluffNetworkHandle * handle;
	union FluffData handle_as_data;

	if (fluff_traceback_grab("fluff_network_connect")){
		return -1;
	} else {
		sock = fluff_socket_connect(addr, 0);
		if (!sock){
			fluff_traceback_set("failed to connect");
			return -1;
		}
		handle = handle_new(sock, HandleTypeConnect, self);
		if (!handle){
			fluff_traceback_set("failed to allocate handle");
			fluff_socket_close(sock);
			return -1;
		}
		handle->on_recv = cb;
		handle_as_data.d_ptr = handle;
		fluff_socket_ud_set(sock, handle_as_data);
		if (!(handle->element = fluff_set_element_element_new(
							self->connecting))){
			fluff_traceback_set("allocating set element failed");
			// Don't know what to do if cleanup fails
			fluff_traceback_pause();
			fluff_traceback_resume();
			handle_free(handle);
			return -1;
		}
		fluff_set_element_element_data_set(handle->element, handle_as_data);
		fluff_set_element_add(self->handles, handle->element);
		fluff_traceback_drop();
		return 0;
	}
}

int fluff_network_poll(struct FluffNetwork * self, double timeout){
	struct FluffSocketIter * iter;
	struct FluffSocket * sock, * new_sock;
	struct FluffNetworkHandle * handle, * new_handle;
	union FluffData handle_as_data;
	int r, w, e, res;

	if (fluff_traceback_grab("fluff_network_poll")){
		return -1;
	} else {
		if (check_connecting(self)){
			fluff_traceback_set("check_connecting failed");
			return -1;
		}
		iter = fluff_socket_select(self->sockets, timeout);
		if (!iter){
			fluff_traceback_set("socket select failed");
			return -1;
		}
		while (!(res = fluff_socket_iter_next(iter, &sock, &r, &w, &e)) && sock){
			handle_as_data = fluff_socket_ud_get(sock);
			handle = handle_as_data.d_ptr;
			if (r){
				switch (handle->type) {
					case HandleTypeListen:
						fluff_traceback_pause();
						new_sock = fluff_socket_accept(sock);
						fluff_traceback_resume();
						if (new_sock){
							if ((new_handle = handle_new(
									new_sock, HandleTypeRemote, self))){
								handle_as_data.d_ptr = new_handle;
								fluff_socket_ud_set(new_sock, handle_as_data);
								if (fluff_socket_set_add(
										self->sockets, new_sock, 1, 1)){
									fluff_traceback_set(
											"adding socket to set failed");
									handle_free(new_handle);
									return -1;
								}
								if (handle->on_recv)
									handle->on_recv(new_handle);
							} else {
								fluff_traceback_set(
										"allocating handle failed");
								fluff_socket_close(new_sock);
								return -1;
							}
						}
						break;
					case HandleTypeRemote:
						if (handle->on_recv)
							handle->on_recv(handle);
						break;
					default:
						fluff_traceback_set("strange behavior");
						return -1;
				}
			}
			if (w){
				if (handle->type == HandleTypeRemote){
					attempt_send(handle);
				}
			}
			if (e){
				if (handle->on_error)
					handle->on_error(handle);
				fluff_network_disconnect(handle);
			}
		}
		fluff_socket_iter_free(iter);
		fluff_traceback_drop();
		return res;
	}
}

void fluff_network_del(struct FluffNetwork * self){
	struct FluffSetElementElement * element;
	union FluffData handle_as_data;
	struct FluffNetworkHandle * handle;

	fluff_socket_set_free(self->sockets);
	while((element = fluff_set_element_pop(self->connecting))) {
		handle_as_data = fluff_set_element_element_data_get(element);
		handle = handle_as_data.d_ptr;
		handle_free(handle);
	}
	while((element = fluff_set_element_pop(self->handles))) {
		handle_as_data = fluff_set_element_element_data_get(element);
		handle = handle_as_data.d_ptr;
		handle_free(handle);
	}
	MM->f_free(self);
}

union FluffData fluff_network_handle_get_userdata(struct FluffNetworkHandle * self){
	return self->userdata;
}

void fluff_network_handle_set_userdata(
		struct FluffNetworkHandle * self,
		union FluffData userdata,
		FluffFreeFunction freer){
	self->userdata = userdata;
	self->ud_freer = freer;
}

void fluff_network_handle_set_recv_callback(
		struct FluffNetworkHandle * self, FluffNetworkCallback cb){
	self->on_recv = cb;
}

void fluff_network_handle_set_error_callback(
		struct FluffNetworkHandle * self, FluffNetworkCallback cb){
	self->on_error = cb;
}

ssize_t fluff_network_recv(
		struct FluffNetworkHandle * self, void * buf, size_t len){
	return fluff_socket_recv(self->sock, buf, len);
}

ssize_t fluff_network_send(
		struct FluffNetworkHandle * self, void * buf, size_t len){
	struct BufferNode * node;
	void * data;

	if ((node = MM->f_alloc(bufnode_size))){
		node->offset = 0;
		node->size = len;
		node->next = NULL;
		switch (self->network->send_opt){
			case FluffNetworkSendOptCopy:
				if (!(node->data = data = malloc(len))){
					MM->f_free(node);
					return -1;
				}
				memcpy(data, buf, len);
				break;
			default:
				node->data = buf;
				break;
		}
		if (!self->buffer_head){
			self->buffer_head = self->buffer_tail = node;
		} else {
			self->buffer_tail->next = node;
			self->buffer_tail = node;
		}
		attempt_send(self);
		return 0;
	}
	return -1;
}

void fluff_network_disconnect(struct FluffNetworkHandle * self){
	fluff_set_element_remove(self->network->handles, self->element);
	handle_free(self);
}
