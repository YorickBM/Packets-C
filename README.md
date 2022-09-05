# Packets-C
This repository contains a networking package system for C. This code can be used for Mac, Linux & Windows. The main.c provided within this project shows a test scenario for windows. Please share credit to this repository when using it since it is linced under GNU General Public License v3.0! Feedback, forks and Issues are always welcome and will be solved, taken into account as quickly as possible.

## How does it work?
C socket networking consists of sending strings over the network. Since in large applications just interpreting strings is not most effective way. This can get very messy quickly i coded packets. These packets have an Id identifying the packet that they are, then you can write data to these packets using the functions: `writeInteger(Packet*, int)` & `writeString(Packet*, char*)`. 
On receiving these packets a predefined callback function will be executed giving you the received packet as a paramater & the id of the client who has send the packet. In this callback you can then pop the data outside of the packet using: `popInteger(Packet*)` & `popString(Packet*)`. Note that the order you have writen them is the order you need to pop them!! 

## How do i use it?
### Create/Receive packet
How to use it is straight forward, first in packet_definitions.h you have to define your packet. For example:
```c
#define DISCONNECT_PACKET 0
void disconnect_packet_received(Packet, char*);
```
After defining the packet in this case DISCONNECT_PACKET with id 0 & a callback function we only need to add this packet to our array in the header here:
```c
#define NEW_PACKET_TYPE(TYPE) {TYPE, #TYPE}
static const packetType packets[] = {
	NEW_PACKET_TYPE(DISCONNECT_PACKET), //<--- HERE
	NEW_PACKET_TYPE(PING_PACKET)
};
```
After this has been done, you can simply go into the packet_definitions.c here you register the callback with your packet:
```c
void register_packets() {
	register_callback(&disconnect_packet_received, DISCONNECT_PACKET); //<--- HERE
	register_callback(&ping_packet_received, PING_PACKET);
}
```
After this is done, you just need to implement your callback function that gets executed, in our example we called it: `disconnect_packet_received(Packet packet, char* connection_id)`.

### Send packets
After you have implemented your custom package & registered it, you can simply call it:
```c
Packet packet = create_packet(DISCONNECT_PACKET); //Here we use DISCONNECT_PACKET the definition we created with 0.
writeString(&packet, "Crash!");
writeInteger(&packet, -1);
writeString(&packet, "Occured on: DATE OR SO");

//To send packet to server:
send_packet_server(&packet);

//To send packet to client, you would need to know the client connection id
send_packet_client(&packet, connection_id);
```
Important to note is that the function you create as the callback can pop 2 strings and 1 integer from this packet.
The order you have writen them, is also the order to pop them! If you do not do this, it might give errors, false information or crashes your application.
To read the packet you use:
```c
void disconnect_packet_received(Packet packet, char* connection_id) { //As you can see we got an connection_id here, so it is possible for us to send something back to the client!
  char* type = popString(&packet);
  int number = popInteger(&packet);
  char* date = popString(&packet);
  printf("Client %s has disconnected because: %s with status %d; %s", connection_id, type, number, date);
}
```

### What to include?
To use this code, you just have to include 2 files into your files where you want to use them.
You must include the packet_definitions.h & packet_reader.h. Then you can use the functions as descrdibed above.
