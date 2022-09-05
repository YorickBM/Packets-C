#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include "src/packet_reader.h"
#include "src/packet_definitions.h"

int send_packet_all_client(Packet*);
int send_packet_client(Packet* packet, char* id);
int send_message_client(char* data, char* id);
int send_packet_server(Packet*);
int send_message_server(char*);
unsigned __stdcall listener(void* arg);
unsigned __stdcall message_handler(void* arg);
int create_connection(char*, int);
int delete_connection(char*);

#define MAX_NUM_CLIENTS 80
#define MAX_CLIENT_MSG_LEN 1000
#define SERVER
#define ID "PUT-YOUR-ID-HERE"

#endif
