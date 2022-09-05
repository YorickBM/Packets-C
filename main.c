#include <windows.h>
#include <tchar.h>
#include <conio.h>
#include <strsafe.h>

#include <stdio.h>
#include <time.h>

#include "packet_reader.h"
#include "packet_definitions.h"

#include "/lib/ntk.h"
#include "/lib/socket_client.h"

int keepRunning = 1;
TCHAR szNewTitle[MAX_PATH];

#define WANTED_PINGS 5

//TODO: Check for memmory leaks
int main(int argc, char** argv)
{
	printf("Main started.\n");
	//Start NTK
	startNTK();

	//Setup server/client connection
	if (create_connection("127.0.0.1", 3000) != 0) {
		printf("EXIT-CODE: Could not connect to server/client");
		return -1;
	}

	//Registering function
	init_registery();
	register_packets();

	int indexed = 0;
	int laps = 0;
	while (keepRunning == 1) {
		StringCchPrintf(szNewTitle, MAX_PATH, TEXT("Custom Network Data Packets - By YorickBM"), 0);
		SetConsoleTitle(szNewTitle);
#ifndef SERVER
		indexed += 1;
		if (indexed == 30000) {
			Packet ping_packet = create_packet(PING_PACKET);
			//send_packet_server(&ping_packet);
			indexed = 0;
			laps += 1;
	}
		if (laps > WANTED_PINGS) {
			keepRunning = 0;
		}
#endif
	} //Keep main thread active!

	Packet discon_packet = create_packet(DISCONNECT_PACKET);
#ifdef SERVER
	send_packet_all_client(&discon_packet);
#else
	send_packet_server(&discon_packet);
#endif

	printf("Disconnected!\n");
	Sleep(1000);

	return 0;
}
