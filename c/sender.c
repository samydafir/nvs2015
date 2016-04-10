/* SENDER */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include "sender.h"

#define MAXSIZE 512

int main(int argc, char *argv[]){

    
    //handle wrong number of cmd args
    if(argc != 5){
        handleError("Usage: ./sender <receiver name> <port> <number of packets to send> <message length>");
    }

    //declare required local vars
    int sock, amtSent, sent, step;
    struct sockaddr_in receiver;
    struct hostent *host;
    struct timeval before, after;
    int msgLength;
    char buffer[MAXSIZE];
    char* msg;
    int amtSend;

    msgLength = atoi(argv[4]);

    //create udp ipv4 socket and handle creation error
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0){
        handleError("socket could not be created");
    }
    
    //set sender info: protocol and host and handle host creation error
    receiver.sin_family = AF_INET;
    host = gethostbyname(argv[1]);
    if(host == 0){
        handleError("host not recognized");
    }

    //copy host address into sockaddr_in struct -> add to addr info and set port
    bcopy((char *)host->h_addr, (char *)&receiver.sin_addr, host->h_length);
    receiver.sin_port = htons(atoi(argv[2]));    
    gettimeofday(&before, NULL);
    for(amtSent = 0; amtSent < atoi(argv[3]); amtSent++){
        //zero out buffer
        bzero(buffer, MAXSIZE);
        //set message to be sent
        sprintf(buffer, "%d: %s", amtSent,msg);
        //send message in buffer as datagram and handle error
        sent = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&receiver, sizeof(struct sockaddr_in));
        if(sent == 0){
            handleError("package not sent successfully");
        }
    }
    gettimeofday(&after, NULL);
    printf("%lums\n", (after.tv_usec - before.tv_usec)/1000 +(after.tv_sec - before.tv_sec)*1000);
    
    return 0;
}

void evaluate(timeval before, timeval after, ){

}

char* createMsg(int length){
    
} 

/*
handleError takes a error message as pointer to a char sequence prints it and exits the application
*/
void handleError(char *message){
    puts(message);
    exit(1);

}
