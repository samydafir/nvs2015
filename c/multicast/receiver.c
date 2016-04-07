/**
 * Receiver represents a UDP packet receiver with multicast capabilities: several receivers can join
 * the multicast group and receive packets intended for the multicast address.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "receiver.h"

#define MAXSIZE 512
#define GROUP_IP "230.0.0.1"

int main(int argc, char *argv[]){

    //requied local var declaration
    int sock, sockaddr_len, one, t_out, rec_cnt;
    struct sockaddr_in receiver, sender;
    struct ip_mreq multi;
    struct timeval timeout;
    char buffer[MAXSIZE];    
    bzero(&receiver, sizeof(receiver));
    bzero(&multi, sizeof(receiver));

    //create udp ipv4 socket
    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0){
        handle_error("socket could not be created");
    }

    //Allow multiple receivers to use same socket
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&one, sizeof(one));
    
    //set address options, like protocol, socket,...
    receiver.sin_family = AF_INET;
    receiver.sin_addr.s_addr = htonl(INADDR_ANY);
    receiver.sin_port = htons(atoi(argv[1]));
    
    //configure timeout for socket
    timeout.tv_sec = atoi(argv[2]);
    timeout.tv_usec = 0;
    if(setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0){
        handle_error("could not set socket receive-timeout");
    }

    //bind socket to created address
    bind(sock,(struct sockaddr*)&receiver,sizeof(receiver));

    //set a multicast address and join socket to the group for that address
    multi.imr_multiaddr.s_addr = inet_addr(GROUP_IP);
    multi.imr_interface.s_addr = htonl(INADDR_ANY);
    setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(const void *)&multi, sizeof(struct ip_mreq));

    //calculate sizeof sockaddr struct
    sockaddr_len = sizeof(struct sockaddr_in);

    //enter while loop and receive packets from the socket and save them into the buffer
    rec_cnt = 0;
    while(recvfrom(sock,buffer,MAXSIZE,0,(struct sockaddr*)&sender,&sockaddr_len) >= 0){
        rec_cnt++;
    }
    printf("%d packets received \n", rec_cnt);
    close(sock);
}

void handle_error(char *message){
    puts(message);
    exit(1);
}
