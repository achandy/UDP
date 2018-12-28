#ifndef Packet
#define Packet

struct packet
{
	short int count;
	short int sequence;
	char data[80];
};


#endif
