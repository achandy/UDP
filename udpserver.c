/* udp_server.c */
/* Programmed by Adarsh Sethi */
/* Sept. 13, 2018 */

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */
#include "packet.h"
#include <time.h>

#define STRING_SIZE 1024

/* SERV_UDP_PORT is the port number on which the server listens for
   incoming messages from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_UDP_PORT 65200

double packetLoss =0;
double ACKLoss = 0;
int lastPacketSeq = 1;
int totalPackets = 0;
int uniquePackets = 0;
int lostPackets = 0;
int duplicatePackets = 0;
int totalBytes = 0;
int totalACKs = 0;
int goodACKs = 0;
int lostACKs = 0;



int main(int argc, char* argv[]) {
   //Opening file to write output to
   FILE *output;
   output = fopen("output.txt", "w");

   //Seed for randomm number
   srand(time(NULL));
   int randNum = rand()%999+65000;
   
   //Checks for correct number of arguments
   if(argc != 3){
	   printf("Not a valid number of arguments\n");
	   exit(1);
   }
   else{
	   packetLoss = atof(argv[1]);
	   ACKLoss = atof(argv[2]);
   }
   
   
 
   int sock_server;  /* Socket on which server listens to clients */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   unsigned short server_port;  /* Port number used by server (local port) */

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned int client_addr_len;  /* Length of client address structure */

   char sentence[STRING_SIZE];  /* receive message */
   char modifiedSentence[STRING_SIZE]; /* send message */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   unsigned int i = 0;  /* temporary loop variable */
   short int ACK=0;

   /* open a socket */

   if ((sock_server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("Server: can't open datagram socket\n");
      exit(1);
   }

   /* initialize server address information */

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */
   server_port = randNum
; /* Server will listen on this port */
   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */

   if (bind(sock_server, (struct sockaddr *) &server_addr,
                                    sizeof (server_addr)) < 0) {
      perror("Server: can't bind to local address\n");
      close(sock_server);
      exit(1);
   }

   /* wait for incoming messages in an indefinite loop */

   printf("Waiting for incoming messages on port %hu\n\n", 
                           server_port);

   client_addr_len = sizeof(client_addr);

   //Declaring instance of struct for received packet
   struct packet recdPacket;
   //Array of null characters for restarting later variable fields
   char nullCharArray[80]={'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
   
	
 while(1){
     (void)strncpy(recdPacket.data,nullCharArray,80);
     bytes_recd = recvfrom(sock_server, &recdPacket, 84, 0,
                     (struct sockaddr *) &client_addr, &client_addr_len);
      printf("\nString length of packet is: %d", (int)strlen(recdPacket.data));
      
      //End of Packet Transmission
      if(recdPacket.count == 0){
	break;
      }
      totalPackets++;

      //If packet is lost
      if(SimulateLoss(packetLoss) == 0){
		printf("\nThe received packet has been lost: %d",recdPacket.sequence);
		lostPackets++;
		printf("\nPacket %d lost", recdPacket.sequence);
		continue; 
      } 
      
      //If packet is received
      if(recdPacket.count != 0){
	if(lastPacketSeq != recdPacket.sequence){
	    uniquePackets++;
            for(i; i < recdPacket.count; i++){
                fputc(recdPacket.data[i],output);
            }
		i= 0;
		totalBytes += recdPacket.count;
		printf("\nPacket %d received with %d data bytes", recdPacket.sequence, recdPacket.count);
		lastPacketSeq = recdPacket.sequence;
	}
	else if(lastPacketSeq == recdPacket.sequence){
		duplicatePackets++;	    
		printf("\nDuplicate packet %d received with %d data bytes", recdPacket.sequence, recdPacket.count);
	}	
      }
	  

     /*printf("Received Sentence is: %s\n     with length %d\n\n",
                         sentence, bytes_recd);*/
      //printf("\nThe received packet has data: %s",recdPacket.data);
      //printf("\nThe received packet has sequence: %d",recdPacket.sequence);

      /* prepare the message to send */

      msg_len = bytes_recd;

     
      
      /* send message */
       if(SimulateACKLoss(ACKLoss) == 1){
	       printf("\nACK %d received", recdPacket.sequence);
               ACK=recdPacket.sequence;
               bytes_sent = sendto(sock_server, &ACK, msg_len, 0,
               (struct sockaddr*) &client_addr, client_addr_len);    
	       totalACKs++;  
	}
       else{
		totalACKs++;	
		lostACKs++;
		printf("\nACK %d lost", recdPacket.sequence);
	}
    }
	fclose(output);
	printf("\nEnd of Transmission Packet with sequence number %d transmitted with %d data bytes\n",recdPacket.sequence,0);
	printf("\n\nNumber of data packets received: %d\nNumber of data bytes received successfully: %d\nTotal number of duplicate packets: %d\nNumber of packets lost: %d\nTotal number of data packets received %d\nSuccesfull ACKs: %d\nLost Acks: %d\nTotal number of ACKs: %d\n\n",
	uniquePackets, totalBytes, duplicatePackets, lostPackets, totalPackets, goodACKs, lostACKs, totalACKs);		
	exit(1);
}


//Function to Simulate Packet Loss
int SimulateLoss(double packetLoss){
	double randNum = (double)rand() / (double)RAND_MAX;
	if (randNum < packetLoss){
		return 0;
	}
	else if (randNum > packetLoss){
		return 1;
	}
	
}

//Function to Simulate ACK Loss
int SimulateACKLoss(double ACKLoss){
	double randNum = (double)rand() / (double)RAND_MAX;
	if (randNum < ACKLoss){
		return 0;
	}
	else if (randNum > ACKLoss){
		return 1;
	}
	
}
	
