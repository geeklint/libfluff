#ifndef FLUFF_NETWORK_H_
#define FLUFF_NETWORK_H_

#include <stddef.h>

#include "data.h"
#include "socket.h"

/*
 * Network object
 */
struct FluffNetwork;

/*
 * Handle object
 */
struct FluffNetworkHandle;

/*
 * Handler function prototype
 */
typedef void (*FluffNetworkCallback)(struct FluffNetworkHandle *);


enum FluffNetworkSendOpt {
    FluffNetworkSendOptNone,
    FluffNetworkSendOptCopy,
    FluffNetworkSendOptFree,
};

/*
 * Create a new network instance
 * Send Options:
 * - None: Buffers will not be freed
 * - Copy: Buffers will be copied and the copy will be freed (default)
 * - Free: This function takes an additional arg (*free)(void *)
 *       which will be called to free the Buffer
 */
struct FluffNetwork * fluff_network_new(enum FluffNetworkSendOpt, ...);

/*
 * Bind to a service and start accepting connections
 * The callback will be called when a once each time a client connects
 * and should setup recv and error callbacks
 */
int fluff_network_bind(
		struct FluffNetwork *,
		struct FluffSocketAddr *,
		FluffNetworkCallback);

/*
 * Connect to a remote service
 * The callback will be called once when a connection is successfully
 * established and should setup recv and error callbacks
 */
int fluff_network_connect(
		struct FluffNetwork *,
		struct FluffSocketAddr *,
		FluffNetworkCallback);

/*
 * Poll for events
 * A timeout of less than 0 causes indefinite blocking
 */
int fluff_network_poll(struct FluffNetwork *, double timeout);

/* Close server */
void fluff_network_del(struct FluffNetwork *);

/*
 * Get the userdata associated with this handle
 */
union FluffData fluff_network_handle_get_userdata(struct FluffNetworkHandle *);

/*
 * Set the userdata associated with this handle
 */
void fluff_network_handle_set_userdata(
		struct FluffNetworkHandle *,
		union FluffData,
		FluffFreeFunction freer);

/*
 * Set the recv callback for this handle. This will be called each time there
 * is data to be received
 */
void fluff_network_handle_set_recv_callback(
		struct FluffNetworkHandle *, FluffNetworkCallback);

/*
 * Set the error callback for this handle. This will be called each time there
 * is an error on the handle. The handle will be closed after this call (this
 * function should not close the handle)
 */
void fluff_network_handle_set_error_callback(
		struct FluffNetworkHandle *, FluffNetworkCallback);

/* Receive data from a handle */
long int fluff_network_recv(
    struct FluffNetworkHandle *, void * buf, size_t len);

/* Send data to a handle */
long int fluff_network_send(
		struct FluffNetworkHandle *, void * buf, size_t len);

/* Disconnect and close handle */
void fluff_network_disconnect(struct FluffNetworkHandle *);

/*
 * Set network module to use fluff_mm
 */
void fluff_network_use_mm(int use);

#endif /* FLUFF_NETWORK_H_ */
