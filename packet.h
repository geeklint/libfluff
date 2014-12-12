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

#ifndef FLUFF_PACKET_H_
#define FLUFF_PACKET_H_

#include "data.h"
#include "socket.h"

/*
 * Packet type
 */
struct FluffPacket;

/*
 * Packet definition object
 */
struct FluffPacketDefinition;

/*
 * Handle object
 */
struct FluffPacketHandle;

/*
 * Client object
 */
struct FluffPacketClient;

typedef (*FluffPacketConnectionHandler)(
		union FluffData, struct FluffPacketHandle *);

typedef (*FluffPacketConnectionFailedHandler)(union FluffData);

typedef (*FluffPacketHandler)(
		union FluffData, struct FluffPacketHandle *, struct FluffPacket *);

/*
 * Create a new packet definition collection
 * Return the new object on success, NULL on failure
 */
struct FluffPacketDefinition * fluff_packet_definition_new();

/*
 * Load packet definitions from file
 * Return 0 on success, -1 on failure
 */
int fluff_packet_load(struct FluffPacketDefinition *, char * filename);

/*
 * Add a single packet definition
 * Return 0 on success, -1 on failure
 */
int fluff_packet_add(
		struct FluffPacketDefinition *, char id, char * def, int c, int s);

/*
 * Set the packet handler for the handle
 */
void fluff_packet_set_handler(
		struct FluffPacketHandle *, FluffPacketHandler, void * data);

/*
 * Create a new server
 * If this function returns, there was an error
 */
void fluff_packet_server(
		struct FluffPacketDefinition *,
		struct FluffSocketAddr *,
		FluffPacketConnectionHandler,
		void * data);

/*
 * Create a new client
 * Return the new client on success, NULL on failure
 */
struct FluffPacketClient * fluff_packet_client();

/*
 * Connect to a host
 * Return 0 on success, -1 on failure
 */
int fluff_packet_connect(
		struct FluffPacketClient *,
		struct FluffSocketAddr *,
		FluffPacketConnectionHandler,
		FluffPacketConnectionFailedHandler,
		void * data);

/*
 * Run client
 * Return 0 on success, -1 on failure
 */
int fluff_packet_client_run(struct FluffPacketClient *);


#endif /* FLUFF_PACKET_H_ */
