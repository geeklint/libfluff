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

#include "packet.h"

struct FluffPacket {
	struct FluffPacketDefinition * definition;
	struct FluffStruct * format;
	void * buffer;
};

struct FluffPacketDefinition {
	struct FluffPacket packets[256];
};

struct FluffPacketHandle {
	struct FluffPacketDefinition * definition;
	FluffPacketHandler handler;
	union FluffData extra;
};

struct FluffPacketClient;

/*
 * Memory manager
 */
static int mm_need_setup = 1;
static const struct FluffMM * MM = NULL;

static union FluffData def_size;

static void setup_mm(){
	if (MM == NULL){
		MM = fluff_mm_default;
	}
    def_size = MM->f_type_new(sizeof(struct FluffPacketDefinition));
    mm_need_setup = 0;
}

#define ENSURE_MM if (mm_need_setup) setup_mm();

/*
 * Create a new packet definition collection
 * Return the new object on success, NULL on failure
 */
struct FluffPacketDefinition * fluff_packet_definition_new(){
	struct FluffPacketDefinition * self;
	int i;

	ENSURE_MM;

	if ((self = MM->f_alloc(def_size))){
		for (i = 0; i < 256; ++i){
			self->packets[i].definition = self;
		}
	}
	return self;
}

int fluff_packet_add(
		struct FluffPacketDefinition * def,
		uint8_t id,
		struct FluffStruct * format){
	void * buffer;

	if ((buffer = MM->f_alloc_size(fluff_struct_size(format)))){
		def->packets[id].format = format;
		def->packets[id].buffer = buffer;
		return 0;
	}
	return -1;
}

int fluff_packet_send(struct FluffPacketHandle * self, uint8_t id, ...){
	va_list args;
	int res;

	va_start(args, id);
	res = fluff_packet_send_v(self, id, args);
	va_end(args);
	return res;
}

int fluff_packet_send_n(struct FluffPacketHandle * self, uint8_t id, ...){
	va_list args;
	int res;

	va_start(args, id);
	res = fluff_packet_send_vn(self, id, args);
	va_end(args);
	return res;
}

int fluff_packet_send_v(
		struct FluffPacketHandle * self, uint8_t id, va_list args){
	struct FluffPacket * packet;

	packet = (self->definition->packets + id);
	fluff_struct_pack_v(packet->format, packet->buffer, args);
	// TODO send buffer
	return 0;
}

int fluff_packet_send_vn(
		struct FluffPacketHandle * self, uint8_t id, va_list args){
	struct FluffPacket * packet;

	packet = (self->definition->packets + id);
	fluff_struct_pack_vn(packet->format, packet->buffer, args);
	// TODO send buffer
	return 0;
}

int fluff_packet_send_s(
		struct FluffPacketHandle * self, uint8_t id, void * struct_){
	struct FluffPacket * packet;

	packet = (self->definition->packets + id);
	fluff_struct_pack_s(packet->format, packet->buffer, struct_);
	// TODO send buffer
	return 0;
}

int fluff_packet_send_sn(
		struct FluffPacketHandle * self, uint8_t id, void * struct_){
	struct FluffPacket * packet;

	packet = (self->definition->packets + id);
	fluff_struct_pack_sn(packet->format, packet->buffer, struct_);
	// TODO send buffer
	return 0;
}

/*
 * Get the packet id from packet
 */
uint8_t fluff_packet_packetid(struct FluffPacket * packet){
	struct FluffPacketDefinition * def;

	def = packet->definition;
	return packet - def->packets;
}

/*
 * Read a packet
 */
void fluff_packet_read(struct FluffPacket * self, ...){
	va_list args;

	va_start(args, self);
	fluff_packet_read_v(self, args);
	va_end(args);
}

void fluff_packet_read_n(struct FluffPacket * self, ...){
	va_list args;

	va_start(args, self);
	fluff_packet_read_vn(self, args);
	va_end(args);
}

void fluff_packet_read_v(struct FluffPacket * self, va_list args){
	fluff_struct_unpack_v(self->format, self->buffer, args);
}

void fluff_packet_read_vn(struct FluffPacket * self, va_list args){
	fluff_struct_unpack_vn(self->format, self->buffer, args);
}

void fluff_packet_read_s(struct FluffPacket * self, void * struct_){
	fluff_struct_unpack_s(self->format, self->buffer, struct_);
}

void fluff_packet_read_sn(struct FluffPacket * self, void * struct_){
	fluff_struct_unpack_sn(self->format, self->buffer, struct_);
}

/*
 * Set the packet handler for the handle
 */
void fluff_packet_set_handler(
		struct FluffPacketHandle * self,
		FluffPacketHandler handler,
		union FluffData extra){
	self->handler = handler;
	self->extra = extra;
}

/*
 * Create a new server
 * If this function returns, there was an error
 */
void fluff_packet_server(
		struct FluffPacketDefinition * self,
		struct FluffSocketAddr * addr,
		FluffPacketConnectionHandler handler,
		union FluffData extra){
	// TODO
}

/*
 * Create a new client
 * Return the new client on success, NULL on failure
 */
struct FluffPacketClient * fluff_packet_client(){
	return NULL; // TODO
}

/*
 * Connect to a host
 * Return 0 on success, -1 on failure
 */
int fluff_packet_connect(
		struct FluffPacketClient * self,
		struct FluffSocketAddr * addr,
		FluffPacketConnectionHandler handler,
		FluffPacketConnectionFailedHandler ehandler,
		union FluffData extra){
	return -1; // TODO
}

/*
 * Run client
 * Return 0 on success, -1 on failure
 */
int fluff_packet_client_run(struct FluffPacketClient * self){
	return -1; // TODO
}

void fluff_packet_setmm(const struct FluffMM * mm){
	MM = mm;
	setup_mm();
}
