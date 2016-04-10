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


int main(int argc, char *argv[]){


    //handle wrong number of cmd args
    if(argc != 5){
        handle_error("Usage: ./receiver <port> <timeout in s> <number of ecpected packets> <packet payload size>");
    }
    //requied local var declaration
    int sock, sockaddr_len, one, t_out, rec_cnt, payload, total_pack_size;
    struct sockaddr_in receiver, sender;
    struct timeval timeout, before, after;
    bzero(&receiver, sizeof(receiver));
    payload = atoi(argv[4]);
    total_pack_size = payload + 6;
    char buffer[total_pack_size];    
    
    //create udp ipv4 socket
    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0){
        handle_error("socket could not be created");
    }
    
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

    //calculate sizeof sockaddr struct
    sockaddr_len = sizeof(struct sockaddr_in);

    //enter while loop and receive packets from the socket and save them into the buffer
    
    rec_cnt = 0;
    if((recvfrom(sock,buffer,total_pack_size,0,(struct sockaddr*)&sender,&sockaddr_len)) >= 0){
        rec_cnt++;        
    }
    gettimeofday(&before, NULL);
    while(recvfrom(sock,buffer,total_pack_size,0,(struct sockaddr*)&sender,&sockaddr_len) >= 0){
        gettimeofday(&after, NULL);
        rec_cnt++;
    }
    evaluate(before, after, payload, rec_cnt);
    close(sock);
}


void evaluate(struct timeval before, struct timeval after, int msg_size, int amt){
    time_t duration = (after.tv_usec - before.tv_usec)/1000 +(after.tv_sec - before.tv_sec)*1000;
    int total_size = msg_size * amt;
    double speed = total_size/duration;
    printf("%d packets received\n", amt);
    printf("%.2f KB/s\n", speed);

}

void handle_error(char *message){
    puts(message);
    exit(1);
}






























