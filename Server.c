#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <sys/socket.h>

#define MESSAGE_LENGHT 64
#define USER_LENGHT 16
#define BACKLOG 1

//Server side

int sfd,cfd;
int masterpid;

static void signal_close_socket(int);

int main(int argc, char* argv[]) {

 //Init Phase: 
 int ssockfd, csockfd, PORT; //defining socket filedes for both server/client
 char buffer[MESSAGE_LENGHT];
 char myuser[USER_LENGHT] = "server1";
 char otheruser[USER_LENGHT];
 struct sockaddr_in saddr, caddr; //defining server/client address
 struct sigaction act;
 pid_t fpid, childpid = 0, sread = 0, swrite = 0;
 sigset_t set;
 socklen_t slen, clen; //defining server/client address lenght

 //getting the port
 PORT = atoi(argv[1]); 

 //defining the master
 masterpid = fpid = getpid();

 //signal handling
 sigfillset(&set);
 act.sa_handler = &signal_close_socket;
 act.sa_mask = set;
 act.sa_flags = SA_RESTART;	
 sigaction(SIGINT, &act, 0);

 //Socket phase: 
 //checking if the socket is created with success.
 if((ssockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("socket");
	exit(EXIT_FAILURE);
 }

 sfd = ssockfd; //assigning the server filedes to sfd.

 //Defining a name for our socket, then binding it.
 //NAME DEFINING PHASE:
 saddr.sin_family = AF_INET; //adding the family address
 saddr.sin_port = htons(PORT); //converting port from host protocol to network protocol, then assigning to current port.
 saddr.sin_addr.s_addr = htonl(INADDR_ANY); //same as above, but with the loopback address.
 

 //now binding
 slen = sizeof(saddr); //size of the address of the server
 if(bind(ssockfd, (struct sockaddr*)&saddr, slen) == -1) {
	perror("bind");
	exit(EXIT_FAILURE);	
 }

 //defining listening phase
 if(listen(ssockfd, BACKLOG) == -1) {
	perror("listen");
	exit(EXIT_FAILURE);	

 }

 printf("Server: correctly loaded.\n");
 printf("Choose your username: ");
 scanf("%s",myuser);
 myuser[strlen(myuser)] = '\0';
 printf("Hello %s!\n",myuser);

 //loop forever
 while(1) {
 	printf("Waiting for connections..\n");
	clen = sizeof(caddr);
	//accept a pending connection
	if((csockfd = accept(ssockfd, (struct sockaddr*)&caddr, &clen)) == -1) {
		perror("accept");
		exit(EXIT_FAILURE);
	} else {
		send(csockfd, &myuser, strlen(myuser)+1, 0);
		recv(csockfd, &otheruser, USER_LENGHT, 0);
		printf("Client %s connected.\n", otheruser);
	}
  
	cfd = csockfd; //assigning client filedes to cfd.	

	//creating two process that will manage read and write respectively
	if((childpid = fork()) == 0) {
		sread=getpid();
	} else if (childpid > 0) {
		childpid = 0;
		if((childpid = fork()) == 0) {
			swrite = getpid();
		} else if (childpid < 0) {
			perror("fork");
			exit(EXIT_FAILURE);
		 }	
	  } else {
		perror("fork");
		exit(EXIT_FAILURE);
	   }	

	while(1) {
		if(sread) {
			recv(csockfd, &buffer, MESSAGE_LENGHT, MSG_WAITALL);
			printf("%s: %s\n",otheruser,buffer);	
			fflush(stdin);	
		}

		if(swrite) {
			scanf("%s",buffer);
			send(csockfd, &buffer, MESSAGE_LENGHT, 0);
			fflush(stdin);		
		}		
	}

  }

}


static void signal_close_socket(int signo) {

 if(getpid() == masterpid) {
 	printf("\nClosing server..\n");
 	close(cfd);
 	close(sfd);
	while(wait(NULL) != -1);
 	exit(EXIT_SUCCESS);
 } else exit(EXIT_SUCCESS);	

}



