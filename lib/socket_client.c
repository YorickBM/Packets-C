#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "socket_client.h"

#include <stdio.h>
#include <winsock2.h>
#include <time.h>

#include "/lib/ntk.h"
//Connection information struct, for easy storage
typedef struct cInfo_s {
	SOCKET socket;
	char* id;
	time_t last_ping;
	task* handler;
} ConnectionInfo;
ConnectionInfo* connections[MAX_NUM_CLIENTS]; //Connection storage

//Few windows socket connection variables
WSADATA wsa;
SOCKET s = INVALID_SOCKET;
struct sockaddr_in server, client;
int sockAddrInLength = sizeof(struct sockaddr_in);

int index = 0;
int registery_index = 0;
int server_running = 1;

char clientMessage[MAX_CLIENT_MSG_LEN];
int clientMessageLength;
char* message;

//Create a new connection on IP & port
int create_connection(char* ip, int port) {

	printf("Initialising Winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d\n\n", WSAGetLastError());
		return -1;
	}
	printf("Initialised.\n");

	printf("Creating socket...\n");
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d\n\n", WSAGetLastError());
		return -1;
	}
	printf("Socket created.\n");

	//Set settings
	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

#ifdef SERVER
	printf("Binding socket...\n");
	if (bind(s, (struct sockaddr*)&server, sizeof server) == SOCKET_ERROR)
	{
		printf("Binding socket on port %d failed.\n\n", port);
		return -1;
	}
	printf("Bound socket!\n");

	printf("Initializing connection storage\n");
	for (int i = 0; i < MAX_NUM_CLIENTS; i++) {
		connections[i] = NULL;
	}
	printf("Done!\n");

	//Create thread that listens to connections
	task* tlistener = (task*)malloc(sizeof(task)); // dynamic task object
	create_task(tlistener, listener, NULL, sizeof(int), 0);
#else
	printf("Connecting socket...\n");
	if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		puts("connect error\n\n");
		return -1;
	}
	printf("Connected!\n");

	char* data = ID;
	send_message_server(data);

	//create thread to handle incoming packages
	task* tHandler = (task*)malloc(sizeof(task)); // dynamic task object
	create_task(tHandler, message_handler, &s, sizeof(SOCKET), 0);

#endif

	return 0;
}

//Function to listen for new connections
unsigned __stdcall listener(void* arg) {
	while (server_running) {
		listen(s, MAX_NUM_CLIENTS);
		printf("Waiting for connection...\n");

		SOCKET sock = accept(s, (struct sockaddr*)&client, &sockAddrInLength);
		if (sock == INVALID_SOCKET)
		{
			fprintf(stderr, "Accept failed.\n");
			continue;
		}
		printf("Accept succeeded.\n");

		//Get identifier
		if ((clientMessageLength = recv(sock, clientMessage, sizeof clientMessage, 0)) == SOCKET_ERROR)
		{
			fprintf(stderr, "Recv failed.\n");
			break;
		}
		printf("Recv succeeded.\n");
		clientMessage[clientMessageLength] = NULL; /* Null terminator */

		//Setup connection information & Register
		ConnectionInfo connection = {.socket = sock, .id = clientMessage};
		time(&connection.last_ping);
		printf("Registered connection %s, Status: %d\n", connection.id, registerConnection(&connection));

		//Create thread for connection to handle incoming packages from that connection
		task* tHandler = (task*)malloc(sizeof(task)); // dynamic task object
		create_task(tHandler, message_handler, &connection, sizeof(ConnectionInfo), 0);
	}
	//Close connection!
	closesocket(s);
	WSACleanup();
	terminate_task(arg);
}

unsigned __stdcall message_handler(void* arg) {
	ConnectionInfo connection = (*(ConnectionInfo*)getArgument_task((task*)arg));
	printf("Handler started for %s\n", connection.id);

	do {
		if ((clientMessageLength = recv(connection.socket, clientMessage, sizeof clientMessage, 0)) == SOCKET_ERROR)
		{
			fprintf(stderr, "Recv failed.\n");
			break;
		}
		//printf("Recv succeeded.\n");

		time(&connection.last_ping);
		clientMessage[clientMessageLength] = NULL; /* Null terminator */
		parse_packet(clientMessage, connection.id);
	} while (connection.keepAlive == 1);

	printf("Handler terminated for %s\n", connection.id);
	terminate_task(arg);
}

int send_packet_client(Packet* packet, char* id) {
	return send_message_client(construct_packet(packet), id);
}

int send_message_client(char* data, char* id) {
	int num_id = getIdForConnection(id);
	if (num_id == -1) {
		printf("No connection found for id: %s\n", id);
		return -1;
	}
	ConnectionInfo* cInfo = connections[num_id];

	if (send(cInfo->socket, data, strlen(data), 0) < 0)
	{
		printf("Send failed.\n");
		return -1;
	}
	//printf("Send succeded. %s\n", data);
}

int send_packet_server(Packet* packet) {
	return send_message_server(construct_packet(packet));
}
int send_message_server(char* data) {
	if (send(s, data, strlen(data), 0) < 0)
	{
		printf("Send failed.\n");
		return -1;
	}
	//printf("Send succeded. %s\n", data);
}

int send_packet_all_client(Packet* packet) {
	char* data = construct_packet(packet);
	for (int i = 0; i < MAX_NUM_CLIENTS; i++) {
		if (connections[i] == NULL) continue;
		ConnectionInfo* cInfo = connections[i];

		if (send(cInfo->socket, data, strlen(data), 0) < 0)
		{
			printf("Send failed.\n");
			return -1;
		}
	}
	return 1;
}
//Kill thread & clear memmory
int delete_connection(char* connection_id) {
	int id = getIdForConnection(connection_id);
	if (id == -1) return 0; //Connection not found we are trying to delete
	connections[id]->keepAlive = 0;
	connections[id] = NULL;
	return 1;
}
int getIdForConnection(char* id) {
	for (int i = 0; i < MAX_NUM_CLIENTS; i++) {
		if (connections[i] == NULL) continue;
		if (strcmp(connections[i]->id, id) == 0) return i;
	}
	return -1;
}
int registerConnection(ConnectionInfo* info) { //Add a connection to our storage
	time_t oldest_ping = NULL;
	int oldestId = -1;

	for (int i = 0; i < MAX_NUM_CLIENTS; i++) {
		if (connections[i] == NULL) {
			connections[i] = info;
			return 1; //Found empty storage spot
		}
		else { //Check if this current connection last package is the oldest (Meaning possible disconnected client)
			if (oldest_ping == NULL) {
				oldest_ping = connections[i]->last_ping;
				continue;
			}
			if (connections[i]->last_ping < oldest_ping) {
				oldest_ping = connections[i]->last_ping;
				oldestId = i;
			}
		}
	}
	if (oldestId == -1) return 0;

	connections[oldestId] = info;
	return 2;
}
