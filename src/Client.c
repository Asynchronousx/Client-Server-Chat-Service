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

#define COLOR_YELLOW "\x1b[33m"
#define RESET "\x1b[0m"
#define MESSAGE_LENGHT 64
#define USER_LENGHT 16

//client side

int cfd;
int masterpid;
char DISCONNECTED[USER_LENGHT];

static void signal_close_handler(int);
void EXIT_PROCEDURE(char);

int main(int argc, char * argv[]) {

    //INIT PHASE
    int sockfd, res, PORT;
    char buffer[MESSAGE_LENGHT];
    char myuser[USER_LENGHT];
    char otheruser[USER_LENGHT];
    pid_t fpid, childpid = 0, sread = 0, swrite = 0;
    socklen_t clen;
    sigset_t set;
    struct sockaddr_in CLIENT_ADDRESS;
    struct sigaction act;

    //defining the port
    PORT = atoi(argv[1]);

    //defining the master pid
    masterpid = fpid = getpid();

    //handling
    sigfillset( & set);
    act.sa_handler = & signal_close_handler;
    act.sa_mask = set;
    act.sa_flags = SA_RESTART;
    sigaction(SIGINT, & act, 0);

    //creating a new socket using NETINET TCP/IP protocol
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("ops");
        exit(EXIT_FAILURE);
    }

    //now defining the socket address name
    CLIENT_ADDRESS.sin_family = AF_INET;
    CLIENT_ADDRESS.sin_port = htons(PORT);
    CLIENT_ADDRESS.sin_addr.s_addr = inet_addr("127.0.0.1"); //change the loopback address with your machine server IP 

    //defining the username
    printf("Client: correctly loaded.\n");
    printf("Write /EXIT and press enter anytime to disconnect.\n");
    printf("Choose your username: ");
    fgets(myuser, USER_LENGHT, stdin);
    strtok(myuser, "\n");

    //trying to connect
    printf("Waiting for the server response..\n");

    clen = sizeof(CLIENT_ADDRESS);
    if ((res = connect(sockfd, (struct sockaddr * ) & CLIENT_ADDRESS, clen)) == -1) {
        perror("ops");
        exit(EXIT_FAILURE);
    } else {
        printf("Connection to the server established, Welcome!\n");
        printf("Waiting for an user to chat with..\n");
        recv(sockfd, & otheruser, USER_LENGHT, 0);
        send(sockfd, & myuser, strlen(myuser) + 1, 0);
        printf("Now chatting with: %s\n\n", otheruser);
        fflush(stdin);
    }

    //chat phase
    while (1) {
        //creating two process that will manage read and write respectively
        if ((childpid = fork()) == 0) {
            sread = getpid();
        } else if (childpid > 0) {
            childpid = 0;
            if ((childpid = fork()) == 0) {
                swrite = getpid();
            } else if (childpid < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
        } else {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        //until the user enter /EXIT
        while (1) {
            if (sread) {
                recv(sockfd, & buffer, MESSAGE_LENGHT, MSG_WAITALL);
                if (!strcmp("/EXIT", buffer)) {
                    strcpy(DISCONNECTED, otheruser);
                    EXIT_PROCEDURE('R');
                }
                printf(COLOR_YELLOW "%s: %s" RESET "\n",otheruser, buffer);
                fflush(stdin);
            }

            if (swrite) {
                fgets(buffer, MESSAGE_LENGHT, stdin);
                strtok(buffer, "\n");
                send(sockfd, & buffer, MESSAGE_LENGHT, 0);
                if (!strcmp("/EXIT", buffer)) EXIT_PROCEDURE('W');
                fflush(stdin);
            }

            if (getpid() == masterpid) {
                pause();
            }
        }
    }
}

static void signal_close_handler(int signo) {

    if (getpid() == masterpid) {
        close(cfd);
        while (wait(NULL) != -1);
        exit(EXIT_SUCCESS);
    } else
        exit(EXIT_SUCCESS);
}

void EXIT_PROCEDURE(char who) {
    if (who == 'R') {
        printf("User %s disconnected.\n", DISCONNECTED);
        printf("Now exiting..\n");
        kill(0, SIGINT);
    } else if (who == 'W') {
        printf("Chat correctly terminated.\n");
        printf("Now exiting..\n");
        kill(0, SIGINT);
    }
}
