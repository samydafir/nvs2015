/*
RECEIVER
Please read "Dokumentation_und_Auswertung.pdf" for more precise information
uses zlib.h
compile with:
gcc -o receiver receiver.c -lz
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>
#include "receiver.h"


int main(int argc, char *argv[]){


    //handle wrong number of cmd args
    if(argc != 5){
        handle_error("Usage: ./receiver <port> <timeout in s> <number of ecpected packets> <packet payload size>");
    }
    //required local var declaration
    int LAS;
    int sock, sockaddr_len, one, t_out, rec_cnt, payload, total_pack_size, pack_exp, i;
    struct sockaddr_in receiver, sender;
    struct timeval timeout, before, after;
    bzero(&receiver, sizeof(receiver));
    payload = atoi(argv[4]);
    total_pack_size = payload + 1;
    uint crc;
    crc = 0;
    int buffer[total_pack_size];
    int ack_buffer[1];
    pack_exp = atoi(argv[3]);
    LAS = -1;

    //create udp ipv4 socket
    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0){
        handle_error("socket could not be created");
    }

    //set address options, like protocol, socket,...
    receiver.sin_family = AF_INET;
    receiver.sin_addr.s_addr = htonl(INADDR_ANY);
    receiver.sin_port = htons(atoi(argv[1]));

    /*configure timeout for socket. Only needed to terminate if no packet
    was received */
    timeout.tv_sec = atoi(argv[2]);
    timeout.tv_usec = 0;

    //bind socket to created address
    bind(sock,(struct sockaddr*)&receiver,sizeof(receiver));

    /*calculate size of sockaddr struct. Saved into a variable due to recvfrom requires
    the memory address where sizeof(sockaddr*) is saved*/
    sockaddr_len = sizeof(struct sockaddr_in);

    //enter while loop and receive packets from the socket and save them into the buffer
    rec_cnt = 0;
    if((recvfrom(sock,buffer,sizeof(buffer),0,(struct sockaddr*)&sender,&sockaddr_len)) >= 0){
        crc = crc32(crc, (const void*)buffer, total_pack_size);
        rec_cnt++;
        ack_buffer[0] = buffer[0];
        sendto(sock, ack_buffer, sizeof(ack_buffer), 0, (struct sockaddr *)&sender, sizeof(struct sockaddr_in));
        LAS = ntohl(ack_buffer[0]);
    }

    /*set after first receive such that the receiver waits for the first packet
    indefinitely*/
    if(setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0){
        handle_error("could not set socket receive-timeout");
    }

    //Set starting time. Does not take the time for receiving the first packet into account.
    gettimeofday(&before, NULL);
    while(recvfrom(sock,buffer,total_pack_size,0,(struct sockaddr*)&sender,&sockaddr_len) >= 0){
        rec_cnt++;
        if(rec_cnt < pack_exp){
            crc = crc32(crc, (const void*)buffer, total_pack_size);
        }else{
            crc = crc32(crc, (const void*)buffer, total_pack_size - 1);
        }
        gettimeofday(&after, NULL);
        ack_buffer[0] = buffer[0];
        sendto(sock, ack_buffer, sizeof(ack_buffer), 0, (struct sockaddr *)&sender, sizeof(struct sockaddr_in));
        LAS = ntohl(ack_buffer[0]);
      }
    evaluate(before, after, total_pack_size, rec_cnt, pack_exp, crc);
    close(sock);
}

/*
evaluates the receive-operation. calculates transfer-speed and prints it.
*/
void evaluate(struct timeval before, struct timeval after, int msg_size, int amt, int expected, uint crc){
    time_t duration = (after.tv_usec - before.tv_usec) + (after.tv_sec - before.tv_sec)*1000000;
    long total_size = msg_size * 4 * amt;
    printf("crc32-checksum: %u\n", crc);
    printf("%d packets expected\n", expected);
    printf("%d packets received\n", amt);
    if(duration != 0.0){
        double speed = 8 * total_size/duration;
        printf("%.3f mbit/s\n", speed);
    }else{
        handle_error("measured time too short. Try sending more packets");
    }
}

/*
handles errors. Prints error-message and quits application
*/
void handle_error(char *message){
    puts(message);
    exit(1);
}
