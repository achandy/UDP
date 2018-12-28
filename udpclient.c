/* udp_client.c */
/* Programmed by Adarsh Sethi */
/* Sept. 13, 2018 */

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */
#include <time.h>
#include <math.h>
#include "packet.h"

#define STRING_SIZE 1024

int main(int argc, char* argv[]) {
   
   //instantiating values to save the command line arguments to
   int n = 5; /*given integer value for testing*/
   FILE *fp;

   //checking for the correct number of command line arguments
   if(argc != 3){
	   printf("Not a valid number of arguments\n");
	   exit(1);
   }
   else{
           //saving the parameters
	   n = atoi(argv[1]);
	   fp=fopen(argv[2],"r");
   }


   int sock_client;  /* Socket used by client */

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned short client_port;  /* Port number used by client (local port) */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
   char server_hostname[STRING_SIZE]; /* Server's hostname */
   unsigned short server_port;  /* Port number used by server (remote port) */

   char sentence[STRING_SIZE];  /* send message */
   char modifiedSentence[STRING_SIZE]; /* receive message */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */

   /* open a socket */

   if ((sock_client = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("Client: can't open datagram socket\n");
      exit(1);
   }

   /* Note: there is no need to initialize local client address information
            unless you want to specify a specific local port.
            The local address initialization and binding is done automatically
            when the sendto function is called later, if the socket has not
            already been bound.
            The code below illustrates how to initialize and bind to a
            specific local port, if that is desired. */

   /* initialize client address information */

   client_port = 0;   /* This allows choice of any available local port */

   /* Uncomment the lines below if you want to specify a particular
             local port: */
   /*
   printf("Enter port number for client: ");
   scanf("%hu", &client_port);
   */

   /* clear client address structure and initialize with client address */
   memset(&client_addr, 0, sizeof(client_addr));
   client_addr.sin_family = AF_INET;
   client_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* This allows choice of
                                        any host interface, if more than one
                                        are present */
   client_addr.sin_port = htons(client_port);

   /* bind the socket to the local client port */

   if (bind(sock_client, (struct sockaddr *) &client_addr,
                                    sizeof (client_addr)) < 0) {
      perror("Client: can't bind to local address\n");
      close(sock_client);
      exit(1);
   }

   /* end of local address initialization and binding */

   /* initialize server address information */

   printf("Enter hostname of server: ");
   scanf("%s", server_hostname);
   if ((server_hp = gethostbyname(server_hostname)) == NULL) {
      perror("Client: invalid server hostname\n");
      close(sock_client);
      exit(1);
   }
   printf("Enter port number for server: ");
   scanf("%hu", &server_port);

   /* Clear server address structure and initialize with server address */
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
                                    server_hp->h_length);
   server_addr.sin_port = htons(server_port);

   /*initialize variables*/
   char * line = NULL;
   size_t len = 0;
   ssize_t read;

   /*timing values initialized*/
   int secTimeout = pow(10,n)/1000000;
   int microsecTimeout = ((int)((int)pow(10,n)%1000000));
   struct timeval timeout;
   timeout.tv_sec=secTimeout;
   timeout.tv_usec=microsecTimeout;
   struct timeval startTime,currTime,waitTime;

   //timeout set as an option on the socket so that the recvfrom will have a timeout setting
   if(setsockopt(sock_client,SOL_SOCKET,SO_RCVTIMEO,(char *) &timeout,sizeof(timeout))<0){
      error("Set socket option failed.\n");
      exit(1);
   }

   /*packet values initialized*/
   struct packet sendPacket;
   short int sequence = 0;
   short int recvACK = -1;
   short int exitACKLoop = 0;

   /*statistics values initialized*/
   int totNumUniquePackets = 0;
   int totNumDataBytes = 0;
   int totNumRetrans = 0;
   int totNumPackets = 0;
   int totNumACKS = 0;
   int totNumTimeouts = 0;

   char nullCharArray[80]={'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
   
   if (fp == NULL)
       exit(EXIT_FAILURE);

   /*loop for wait for call from above, reads in next line if there is one,
   aka recieves call from above if there is another line to send*/
   while ((read = getline(&line, &len, fp)) != -1) {
     sendPacket.count = -1;
     (void)strncpy(sendPacket.data,nullCharArray,80);
  
     /*make a packet from read values*/
     sendPacket.count = read;
     sendPacket.sequence = sequence;
     (void)strncpy(sendPacket.data,line,80);
    
     /*transmit the line with no null characters included*/
     bytes_sent = sendto(sock_client, &sendPacket, (int)strlen(sendPacket.data)+4, 0,
              (struct sockaddr *) &server_addr, sizeof (server_addr));//(int)strlen(sendPacket.data)
     printf("Packet %d transmitted with %d data bytes\n",sendPacket.sequence,sendPacket.count);
     totNumUniquePackets++;
     totNumPackets++;
     totNumDataBytes+=sendPacket.count;
     
     gettimeofday(&startTime,NULL);

     /*loop waiting for first thing to occur, ACK recieved or timeout*/
     exitACKLoop=0;
     bytes_recd=0;

     //loop until ACK is received
     while (exitACKLoop==0){
       bytes_recd = recvfrom(sock_client, &recvACK, 2, 0,(struct sockaddr *) 0, (int *) 0);
       gettimeofday(&currTime,NULL);

       //if a timeout is the first that occurred
       if (((currTime.tv_sec*1000000-startTime.tv_sec*1000000)+(currTime.tv_usec-startTime.tv_usec))>=(timeout.tv_sec*1000000+timeout.tv_usec)){
         printf("Timeout expired for packet numbered %d\n",sendPacket.sequence);
         totNumTimeouts++;
         bytes_sent = sendto(sock_client, &sendPacket, (int)strlen(sendPacket.data)+4, 0,
                  (struct sockaddr *) &server_addr, sizeof (server_addr));
         printf("Packet %d retransmitted with %d data bytes\n",sequence,sendPacket.count);
         totNumRetrans++;
         totNumPackets++;
         gettimeofday(&startTime,NULL);
       }  
       //if the client received the ACK before a timeout     
       else if(bytes_recd>0){
         totNumACKS++;
         //if the ACK is of the expected sequence 
         if(recvACK==sequence){
           printf("ACK %d received\n",recvACK);
           exitACKLoop=1;
           sequence = (sequence+1)%2;
         }
         //if the ack received was not the one expected
         else{
           printf("AN OUT OF SEQUENCE ACK ARRIVED");
           gettimeofday(&startTime,NULL);
         }
       } 
       bytes_recd=0;
     }//end of inner loop
     
   }//end of outer loop

   /*send end of transmission packet*/
   sendPacket.count = 0;
   sendPacket.sequence = sequence;
   char emptyArr[] = {};
   (void)strncpy(sendPacket.data,emptyArr,0);
   bytes_sent = 0;
   bytes_sent = sendto(sock_client, &sendPacket, 4, 0,
                  (struct sockaddr *) &server_addr, sizeof (server_addr));

   if(bytes_sent>0){
   	printf("End of Transmission Packet with sequence number %d transmitted with %d data bytes\n\n",sequence,0);
   }

   //close the file
   fclose(fp);

   /*print the statistics*/
   printf("Number of Unique Packets Sent: %d\n", totNumUniquePackets);
   printf("Number of Unique Data Bytes Sent: %d\n", totNumDataBytes);
   printf("Number of Retransmissions: %d\n", totNumRetrans);
   printf("Total Number of Packets Sent: %d\n", totNumPackets);
   printf("Number of ACKs Received: %d\n", totNumACKS);
   printf("Number of Timeouts: %d\n\n", totNumTimeouts);



   /* close the socket */
   close (sock_client);
}
