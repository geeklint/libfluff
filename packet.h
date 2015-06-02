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
#include "mm.h"
#include "socket.h"
#include "struct.h"

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

/*
 * Callback functions
 */
typedef void (*FluffPacketConnectionHandler)(
		union FluffData, struct FluffPacketHandle *);

typedef void (*FluffPacketConnectionFailedHandler)(union FluffData);

typedef void (*FluffPacketHandler)(
		union FluffData, struct FluffPacketHandle *, struct FluffPacket *);

/*
 * Create a new packet definition collection
 * Return the new object on success, NULL on failure
 */
struct FluffPacketDefinition * fluff_packet_definition_new();

/*
 * Add a single packet definition
 * Return 0 on success, -1 on failure
 */
int fluff_packet_add(
		struct FluffPacketDefinition *, uint8_t id, struct FluffStruct *);

/*
 * Send a packet to a handle
 */
int fluff_packet_send(struct FluffPacketHandle *, uint8_t id, ...);
int fluff_packet_send_n(struct FluffPacketHandle *, uint8_t id, ...);
int fluff_packet_send_v(struct FluffPacketHandle *, uint8_t id, va_list args);
int fluff_packet_send_vn(struct FluffPacketHandle *, uint8_t id, va_list args);
int fluff_packet_send_s(
		struct FluffPacketHandle *, uint8_t id, void * struct_);
int fluff_packet_send_sn(
		struct FluffPacketHandle *, uint8_t id, void * struct_);

/*
 * Get the packet id from packet
 */
uint8_t fluff_packet_packetid(struct FluffPacket *);

/*
 * Read a packet
 */
void fluff_packet_read(struct FluffPacket *, ...);
void fluff_packet_read_n(struct FluffPacket *, ...);
void fluff_packet_read_v(struct FluffPacket *, va_list args);
void fluff_packet_read_vn(struct FluffPacket *, va_list args);
void fluff_packet_read_s(struct FluffPacket *, void * struct_);
void fluff_packet_read_sn(struct FluffPacket *, void * struct_);

/*
 * Set the packet handler for the handle
 */
void fluff_packet_set_handler(
		struct FluffPacketHandle *, FluffPacketHandler, union FluffData extra);

/*
 * Create a new server
 * If this function returns, there was an error
 */
void fluff_packet_server(
		struct FluffPacketDefinition *,
		struct FluffSocketAddr *,
		FluffPacketConnectionHandler,
		union FluffData extra);

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
		union FluffData extra);

/*
 * Run client
 * Return 0 on success, -1 on failure
 */
int fluff_packet_client_run(struct FluffPacketClient *);

/*
 * Set the memory manager used by the packet submodule
 */
void fluff_packet_setmm(const struct FluffMM *);

#endif /* FLUFF_PACKET_H_ */
