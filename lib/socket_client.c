#include "socket_client.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <winsock2.h>
#include <time.h>

WSADATA wsa;
SOCKET s = INVALID_SOCKET;
ConnectionInfo[MAX_NUM_CLIENTS] connection;
struct sockaddr_in server, client;
int sockAddrInLength = sizeof(struct sockaddr_in);

typedef struct cinfo {
	SOCKET socket;
	char* id;
	time_t last_ping
} ConnectionInfo;

#include "inc/ntk.h"

int registerConnection(ConnectionInfo* info) {
	time_t oldest_ping = NULL;
	int oldestId = -1;

	for (int i = 0; i < MAX_NUM_CLIENTS) {
		if (connection[i] == null) {
			connection[i] = info;
			return 1; //Found empty registery spot
		}
		else {
			if (oldest_ping == NULL) {
				oldest_ping = connection[i].last_ping;
				contineu;
			}
			if (connection[i].last_ping < oldest_ping) {
				oldest_ping = connection[i].last_ping;
				oldestId = i;
			}
		}
	}
	if (oldestId == -1) return 0;

	connection[oldestId] = info;
	return 2;
}

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

	printf("Initializing connection regitsery\n");
	for (int i = 0; i < MAX_NUM_CLIENTS; i++) {
		connection[i] = NULL;
	}
	printf("Done!\n")

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

	//Create thread to handle sockets
	task* tHandler = (task*)malloc(sizeof(task)); // dynamic task object
	create_task(tHandler, message_handler, &s, sizeof(SOCKET), 0);

#endif

	return 0;
}

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

		ConnectionInfo connection;
		connection.socket = sock;
		strcpy(connection.id, clientMessage);
		time(&connection.last_ping);
		printf("Registered connection %f as %d", clientMessage, registerConnection(&connection));

		//Create thread to handle socket
		task* tHandler = (task*)malloc(sizeof(task)); // dynamic task object
		create_task(tHandler, message_handler, &connection, sizeof(ConnectionInfo), 0);
	}
	//Close connection!
	closesocket(s);
	WSACleanup();
	terminate_task(arg);
}

unsigned __stdcall message_handler(void* arg) {
	int client_running = 1;
	ConnectionInfo cInfo = (*(ConnectionInfo*)getArgument_task((task*)arg));

	while (client_running) {
		if ((clientMessageLength = recv(cInfo.socket, clientMessage, sizeof clientMessage, 0)) == SOCKET_ERROR)
		{
			fprintf(stderr, "Recv failed.\n");
			break;
		}
		//printf("Recv succeeded.\n");


		clientMessage[clientMessageLength] = NULL; /* Null terminator */
		parse_packet(clientMessage, cInfo);
	}
	terminate_task(arg);
}


int send_packet_client(Packet* packet, ConnectionInfo cInfo) {
	return send_message_client(construct_packet(packet), cInfo);
}

int send_message_client(char* data, ConnectionInfo cInfo) {
	if (send(cInfo.socket, data, strlen(data), 0) < 0)
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