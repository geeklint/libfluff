#ifndef FLUFF_SOCKET_H_
#define FLUFF_SOCKET_H_

#include <stddef.h>
#include <stdint.h>

#include "data.h"

/*
 * Socket object
 */
struct FluffSocket;

/*
 * Socket Address object
 */
struct FluffSocketAddr;

/*
 * Create a new socket address object
 */
struct FluffSocketAddr * fluff_socket_addr(char * host, uint16_t port);

/*
 * Get host and port from address
 */
void fluff_socket_addr_value(
		struct FluffSocketAddr *, char ** host, uint16_t * port);

/*
 * Bind and start listening on the specified service
 */
struct FluffSocket * fluff_socket_bind(
		struct FluffSocketAddr *, int blocking, int reuse_addr);

/*
 * Connect to remote service
 * If blocking is false, use _connected function to test if a
 */
struct FluffSocket * fluff_socket_connect(
		struct FluffSocketAddr *, int blocking);

/*
 * Test to see if a non-blocking socket is connected yet:
 * less than 0: an error occurred
 * 0: not connected
 * greater than 0: connected
 */
int fluff_socket_connected(struct FluffSocket *);

/*
 * Accept an incoming connection on a bound socket
 */
struct FluffSocket * fluff_socket_accept(struct FluffSocket *);

/*
 * Recv data, same as normal socket recv
 */
long int fluff_socket_recv(struct FluffSocket *, void * buf, size_t size);

/*
 * Send data, same as normal socket send
 */
long int fluff_socket_send(struct FluffSocket *, void * buf, size_t size);

/*
 * Get address of socket
 */
struct FluffSocketAddr * fluff_socket_addr_get(struct FluffSocket *);

/*
 * Getter/Setter for socket userdata
 */
void fluff_socket_ud_set(struct FluffSocket *, union FluffData);
union FluffData fluff_socket_ud_get(struct FluffSocket *);

/*
 * Invalidate and close a socket
 */
void fluff_socket_close(struct FluffSocket *);

/*
 * Socket set object
 */
struct FluffSocketSet;

/*
 * Create a new socket set
 */
struct FluffSocketSet * fluff_socket_set_new();

/*
 * Add a socket to the set
 * Returns 0 on success -1 on error
 */
int fluff_socket_set_add(
		struct FluffSocketSet *, struct FluffSocket *, int read, int write);

/*
 * Modify a socket in the set
 * Returns 0 on success -1 on error
 */
int fluff_socket_set_mod(
		struct FluffSocketSet *, struct FluffSocket *, int read, int write);

/*
 * Remove a socket from the set
 * Returns 0 on success -1 on error
 */
int fluff_socket_set_del(struct FluffSocketSet *, struct FluffSocket *);

/*
 * Free a set object
 */
void fluff_socket_set_free(struct FluffSocketSet *);

/*
 * Socket iter object
 */
struct FluffSocketIter;

/*
 * Check for events on the sockets
 */
struct FluffSocketIter * fluff_socket_select(
		struct FluffSocketSet *, double timeout);

/*
 * Get the next socket from the iter
 */
int fluff_socket_iter_next(
		struct FluffSocketIter *,
		struct FluffSocket **,
		int * read, int * write, int * error);

/*
 * Free the iterator
 */
void fluff_socket_iter_free(struct FluffSocketIter *);

/*
 * Define whether this module should use fluff_mm
 */
void fluff_socket_use_mm(int use);

#endif /* FLUFF_SOCKET_H_ */
