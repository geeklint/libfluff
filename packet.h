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
 */
struct FluffPacketDefinition * fluff_packet_definition_new();

/*
 * Load packet definitions from file
 */
int fluff_packet_load(struct FluffPacketDefinition *, char * filename);

/*
 * Add a single packet definition
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
 */
struct FluffPacketClient * fluff_packet_client();

/*
 * Connect to a host
 */
int fluff_packet_connect(
		struct FluffPacketClient *,
		struct FluffSocketAddr *,
		FluffPacketConnectionHandler,
		FluffPacketConnectionFailedHandler,
		void * data);

/*
 * Run client
 */
int fluff_packet_client_run(struct FluffPacketClient *);


#endif /* FLUFF_PACKET_H_ */
