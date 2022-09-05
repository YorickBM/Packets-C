/*******************************************************
* Authors: Yorick Blom
* Version: 1.0
* Last modification: 05-09-2022
********************************************************/

#define _CRT_SECURE_NO_WARNINGS

#include "packet_definitions.h"
#include "/lib/socket_client.h"

#include <stdio.h>
#include <time.h>

void register_packets() {

	//Register packets
	register_callback(&disconnect_packet_received, DISCONNECT_PACKET);
	register_callback(&ping_packet_received, PING_PACKET);
}

void disconnect_packet_received(Packet packet, char* connection_id) {
#ifdef SERVER
	printf("Client %s has disconnected! Status: %d\n", connection_id, delete_connection(connection_id));
#else
	printf("Server has disconnected!\n");
#endif
}

void ping_packet_received(Packet packet, char* connection_id) {
#ifdef SERVER
	printf("PING\n");
	Packet nPacket = create_packet(PING_PACKET);
	send_packet_client(&nPacket, connection_id);
#else
	printf("PONG\n");
#endif
}

void log_action(Packet packet, char* connection_id) {
	int actions_to_log = popInteger(&packet);
	
	for(int i = 0; i < actions_to_log; i++) {
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		
		char* data = popString(&packet);
		
		printf("[%02d:%02d:%02d] - %s\n", tm.tm_hour, tm.tm_min, tm.tm_sec, data);
	}
}
