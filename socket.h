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
 * Return the new object on success, NULL on failure
 */
struct FluffSocketAddr * fluff_socket_addr(char * host, uint16_t port);

/*
 * Get host and port from address
 */
void fluff_socket_addr_value(
		struct FluffSocketAddr *, char ** host, uint16_t * port);

/*
 * Bind and start listening on the specified address
 * Return the bound socket on success, NULL on failure
 */
struct FluffSocket * fluff_socket_bind(
		struct FluffSocketAddr *, int blocking, int reuse_addr);

/*
 * Connect to remote service
 * If blocking is false, use _connected function to test if a socket
 * connection has succeeded yet
 * Blocking: Return the connected socket on success, NULL on failure
 * Non-Blocking: Return a new socket on success, NULL on failure
 */
struct FluffSocket * fluff_socket_connect(
		struct FluffSocketAddr *, int blocking);

/*
 * Test to see if a non-blocking socket is connected yet:
 * Return 1 if connected, 0 if not connected and -1 on failure
 */
int fluff_socket_connected(struct FluffSocket *);

/*
 * Accept an incoming connection on a bound socket
 * Return the accepted socket on success, NULL on failure
 */
struct FluffSocket * fluff_socket_accept(struct FluffSocket *);

/*
 * Receive data from socket
 * Return size of data received on success, -1 on failure
 */
long int fluff_socket_recv(struct FluffSocket *, void * buf, size_t size);

/*
 * Send data to socket
 * Return size of data sent on success, -1 on failure
 */
long int fluff_socket_send(struct FluffSocket *, void * buf, size_t size);

/*
 * Get address of socket
 * Return the address of the socket on success, NULL on failure
 */
struct FluffSocketAddr * fluff_socket_addr_get(struct FluffSocket *);

/*
 * Set userdata associated with a socket
 */
void fluff_socket_ud_set(struct FluffSocket *, union FluffData);

/*
 * Get userdata associated with a socket
 * Return the userdata associated with the socket
 */
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
 * Returns 0 on success, -1 on failure
 */
int fluff_socket_set_add(
		struct FluffSocketSet *, struct FluffSocket *, int read, int write);

/*
 * Modify a socket in the set
 * Returns 0 on success, -1 on failure
 */
int fluff_socket_set_mod(
		struct FluffSocketSet *, struct FluffSocket *, int read, int write);

/*
 * Remove a socket from the set
 * Returns 0 on success, -1 on failure
 */
int fluff_socket_set_del(struct FluffSocketSet *, struct FluffSocket *);

/*
 * Free a socket set object
 */
void fluff_socket_set_free(struct FluffSocketSet *);

/*
 * Socket iter object
 */
struct FluffSocketIter;

/*
 * Check for events on the sockets
 * Return sockets with events on success, NULL on failure
 */
struct FluffSocketIter * fluff_socket_select(
		struct FluffSocketSet *, double timeout);

/*
 * Get the next socket from the iter
 * Return 0 on succes, -1 on failure
 */
int fluff_socket_iter_next(
		struct FluffSocketIter *,
		struct FluffSocket **,
		int * read, int * write, int * error);

/*
 * Free the iterator
 */
void fluff_socket_iter_free(struct FluffSocketIter *);

#endif /* FLUFF_SOCKET_H_ */
