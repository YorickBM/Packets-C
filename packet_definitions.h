/*******************************************************
* Authors: Yorick Blom
* Version: 1.0
* Last modification: 05-09-2022
********************************************************/

#ifndef PACKET_DEFINITIONS_H
#define PACKET_DEFINITIONS_H

#include "packet_reader.h"
void register_packets();
typedef struct struct_packetType {
	int id;
	const char* type;
} packetType;

#define DISCONNECT_PACKET 0
void disconnect_packet_received(Packet, char*);

#define PING_PACKET 1
void ping_packet_received(Packet, char*);

#define NEW_PACKET_TYPE(TYPE) {TYPE, #TYPE}
static const packetType packets[] = {
	NEW_PACKET_TYPE(DISCONNECT_PACKET),
	NEW_PACKET_TYPE(PING_PACKET)
};

#endif
