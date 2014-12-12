#include "network.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "mm.h"
#include "set.h"

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
 * Utility (memory management)
 */

static struct FluffMMGroup * mm_group = NULL;
static struct FluffMMType * buffernode_size = NULL;
static struct FluffMMType * handle_size = NULL;
static struct FluffMMType * network_size = NULL;

static void setup_mm_types(){
    mm_group = fluff_mm_new_g();
    buffernode_size = fluff_mm_new(sizeof(struct BufferNode), mm_group);
    handle_size = fluff_mm_new(sizeof(struct FluffNetworkHandle), mm_group);
    network_size = fluff_mm_new(sizeof(struct FluffNetwork), mm_group);
}

#define ENSURE_MM if (!mm_group) setup_mm_types();

/*
 * Internal (static) methods and functions
 */

static struct FluffNetworkHandle * handle_new(
		struct FluffSocket * sock,
		enum HandleType type,
		struct FluffNetwork * network){
	struct FluffNetworkHandle * self;

	if ((self = fluff_mm_alloc(handle_size))){
		self->sock = sock;
		self->type = type;
		self->network = network;
		self->userdata = fluff_data_zero;
		self->buffer_head = NULL;
		self->buffer_tail = NULL;
		self->on_recv = NULL;
		self->on_error = NULL;
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
		fluff_mm_free(buffer);
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
	fluff_mm_free(self);
}

static int check_connecting(struct FluffNetwork * self){
	struct FluffNetworkHandle * handle;
	union FluffData handle_as_data;
	struct FluffSetElementElement * element;
	struct FluffSetElementIter * iter;
	int res;

	iter = fluff_set_element_iter(self->connecting);
	if (!iter){
		LOGD("creating iter failed");
		return -1;
	}
	while ((element = fluff_set_element_iter_next(iter))){
		handle_as_data = fluff_set_element_element_data_get(element);
		handle = handle_as_data.d_ptr;
		res = fluff_socket_connected(handle->sock);
		if (res < 0){
			LOGD("check for connection failed");
			res = -1;
			break;
		} else if (res > 0){
			fluff_set_element_iter_removelast(iter);
			fluff_set_element_element_free(element);
			if (!(handle->element = element = fluff_set_element_element_new(
					self->handles))){
				LOGD("creating new element failed");
				res = -1;
				break;
			}
			fluff_set_element_element_data_set(element, handle_as_data);
			handle->type = HandleTypeRemote;
			if (fluff_socket_set_add(self->sockets, handle->sock, 1, 1)){
				LOGD("adding socket to set failed");
				handle_free(handle);
				res = -1;
				break;
			}
			fluff_set_element_add(self->handles, element);
		}
		res = 0;
	}
	fluff_set_element_iter_free(iter);
	return res;
}

/*
 * Attempt to send outgoing data to the client
 */
static int attempt_send(struct FluffNetworkHandle * handle){
    ssize_t res;
    struct BufferNode * node;

    while ((node = handle->buffer_head)){
        res = fluff_socket_send(
            handle->sock,
            node->data + node->offset,
            node->size - node->offset);
        if (res < 0){
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
    return 0;
}

/*
 * Exported Functions
 */

struct FluffNetwork * fluff_network_new(
		enum FluffNetworkSendOpt send_opt, ...){
	struct FluffNetwork * self;
	va_list args;

	ENSURE_MM;

	LOGD("allocating network");
	if ((self = fluff_mm_alloc(network_size))){
		LOGD("network allocated");
		LOGD("allocating sockets");
		if (!(self->sockets = fluff_socket_set_new())){
			fluff_mm_free(self);
			return NULL;
		}
		LOGD("sockets allocated");
		self->send_opt = send_opt;
		if (send_opt == FluffNetworkSendOptFree){
			va_start(args, send_opt);
			self->send_freer = va_arg(args, FluffFreeFunction);
			va_end(args);
		}
		self->connecting = NULL;
		self->handles = NULL;
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

	sock = fluff_socket_bind(addr, 0, 1);
	if (!sock){
		return -1;
	}
	handle = handle_new(sock, HandleTypeListen, self);
	if (!handle){
		fluff_socket_close(sock);
		return -1;
	}
	handle->on_recv = cb;
	handle_as_data.d_ptr = handle;
	fluff_socket_ud_set(sock, handle_as_data);
	if (fluff_socket_set_add(self->sockets, sock, 1, 0)){
		handle_free(handle);
		return -1;
	}
	if (!(handle->element = fluff_set_element_element_new(
						self->handles))){
	}
	return 0;
}

int fluff_network_connect(
		struct FluffNetwork * self,
		struct FluffSocketAddr * addr,
		FluffNetworkCallback cb){
	struct FluffSocket * sock;
	struct FluffNetworkHandle * handle;
	union FluffData handle_as_data;

	sock = fluff_socket_connect(addr, 0);
	if (!sock){
		return -1;
	}
	handle = handle_new(sock, HandleTypeConnect, self);
	if (!handle){
		fluff_socket_close(sock);
		return -1;
	}
	handle->on_recv = cb;
	handle_as_data.d_ptr = handle;
	fluff_socket_ud_set(sock, handle_as_data);
	handle->next = self->connecting;
	self->connecting = handle;
	return 0;
}

int fluff_network_poll(struct FluffNetwork * self, double timeout){
	struct FluffSocketIter * iter;
	struct FluffSocket * sock, * new_sock;
	struct FluffNetworkHandle * handle, * new_handle;
	union FluffData handle_as_data;
	int r, w, e, res;

	LOGD("checking connecting");
	if (check_connecting(self)){
		return -1;
	}
	LOGD("connecting checked");
	LOGD("allocating iter");
	iter = fluff_socket_select(self->sockets, timeout);
	if (!iter){
		return -1;
	}
	LOGD("iter allocated");
	while (!(res = fluff_socket_iter_next(iter, &sock, &r, &w, &e)) && sock){
		LOGD("handling event");
		handle_as_data = fluff_socket_ud_get(sock);
		handle = handle_as_data.d_ptr;
		if (r){
			switch (handle->type) {
				case HandleTypeListen:
					if ((new_sock = fluff_socket_accept(sock))){
						if ((new_handle = handle_new(
								new_sock, HandleTypeRemote, self))){
							handle_as_data.d_ptr = new_handle;
							fluff_socket_ud_set(new_sock, handle_as_data);
							if (fluff_socket_set_add(
									self->sockets, new_sock, 1, 1)){
								handle_free(new_handle);
								continue;
							}
							new_handle->next = self->handles;
							self->handles = new_handle;
							if (handle->on_recv)
								handle->on_recv(new_handle);
						} else {
							fluff_socket_close(new_sock);
						}
					}
					break;
				case HandleTypeRemote:
					if (handle->on_recv)
						handle->on_recv(handle);
					break;
				default:
					break;
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
	return res;
}

void fluff_network_del(struct FluffNetwork * self){
	fluff_socket_set_free(self->sockets);
	handle_free(self->connecting);
	handle_free(self->handles);
	fluff_mm_free(self);
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

	if ((node = fluff_mm_alloc(buffernode_size))){
		node->offset = 0;
		node->size = len;
		node->next = NULL;
		switch (self->network->send_opt){
			case FluffNetworkSendOptCopy:
				if (!(node->data = data = malloc(len))){
					fluff_mm_free(node);
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
	struct FluffNetworkHandle ** prev;

	fluff_set_element_remove(self->network->handles, self->element);
	handle_free(self);
}

/*
 * Set network module to use fluff_mm
 */
void fluff_network_use_mm(int use){
	ENSURE_MM;

	fluff_mm_manage_g(mm_group, use);
}
