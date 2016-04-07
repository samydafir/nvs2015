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

    //define required local vars
    int sock, amtSent, sent, step;
    struct sockaddr_in receiver;
    struct hostent *host;
    char buffer[MAXSIZE];

    //handle wrong number of cmd args
    if(argc != (4 + atoi(argv[3]))){
        handleError("Usage: ./sender <receiver name> <port> <number of sending steps> <number of packages to send in each step>");
    }

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
    
    
    for(step = 4; step < argc; step++){
        printf("sending %d packets...\n", atoi(argv[step]));
        for(amtSent = 0; amtSent < atoi(argv[step]); amtSent++){
            //zero out buffer
            bzero(buffer, 512);
            //set message to be sent
            sprintf(buffer, "%04d: packet received", amtSent);
            //send message in buffer as datagram and handle error
            sent = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&receiver, sizeof(struct sockaddr_in));
            if(sent == 0){
                handleError("package not sent successfully");
            }
        }
    }

    return 0;
}

/*
handleError takes a error message as pointer to a char sequence prints it and exits the application
*/
void handleError(char *message){
    puts(message);
    exit(1);

}
