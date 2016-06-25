/*
SENDER
Please read "Dokumentation_und_Auswertung.pdf" for more precise information
uses zlib.h
compile with:
gcc -o sender sender.c -lz
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <zlib.h>
#include <unistd.h>
#include "sender.h"

int main(int argc, char *argv[]){


    //handle wrong number of cmd args
    if(argc != 7){
        handle_error("Usage: ./sender <receiver name> <port> <number of packets to send> <packet payload size> <ack-timeout> <window-size>");
    }

    //declare and define required local vars
    int LAR;
    int sock, amt_send, amt_sent, sent, total_pack_size, payload, sockaddr_len, i;
    struct sockaddr_in receiver, sender;
    struct hostent *host;
    struct timeval before, after;
    struct timeval ack_timeout, sleep;
    uint crc;
    crc = 0;
    payload = atoi(argv[4]);
    total_pack_size = payload + 1;
    int ack_buffer[1];
    int buffer[total_pack_size];
    for(i = 0; i < total_pack_size; i++){
        buffer[i] = htonl(9);
    }
    LAR = -1;
    amt_send = atoi(argv[3]);
    int window_size;
    window_size = atoi(argv[6]);

    //create udp ipv4 socket and handle creation error
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0){
        handle_error("socket could not be created");
    }

    /*set after first receive such that the receiver waits for the first packet
    indefinitely*/
    ack_timeout.tv_sec = 0;
    ack_timeout.tv_usec = atoi(argv[5]) * 1000;
    if(setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&ack_timeout,sizeof(ack_timeout)) < 0){
        handle_error("could not set socket receive-timeout");
    }


    //set sender info: protocol and host and handle host creation error
    receiver.sin_family = AF_INET;
    host = gethostbyname(argv[1]);
    if(host == 0){
        handle_error("host not recognized");
    }

    //copy host address into sockaddr_in struct -> add to addr info and set port
    bcopy((char *)host->h_addr, (char *)&receiver.sin_addr, host->h_length);
    receiver.sin_port = htons(atoi(argv[2]));
    gettimeofday(&before, NULL);
    //start sending
    for(amt_sent = 0; amt_sent < amt_send; amt_sent++){
        //set sequence number
        buffer[0] = htonl(amt_sent);
        //check cases
        if(amt_sent < amt_send - 1){
            crc = crc32(crc, (const void*)buffer, total_pack_size);
        }else{
            crc = crc32(crc, (const void*)buffer, total_pack_size - 1);
            buffer[total_pack_size - 1] = htonl(crc);
        }
        //send message in buffer as datagram and handle error
        sent = sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&receiver, sizeof(struct sockaddr_in));
        if(sent == 0){
            handle_error("package not sent successfully");
        }
      /*handle acks, go-back-n if either ack not expected ack or no ack received for
      first packet in sliding window */
      if(amt_sent == LAR + window_size){
          if(recvfrom(sock,ack_buffer,sizeof(ack_buffer),0,(struct sockaddr*)&sender,&sockaddr_len) >= 0){
              if(ntohl(ack_buffer[0]) == LAR + 1){
                  LAR = ntohl(ack_buffer[0]);
              }else{
                amt_sent = LAR;
              }
          }else{
            amt_sent = LAR;
          }
      }
    }
    gettimeofday(&after, NULL);
    evaluate(before,after,total_pack_size,amt_send,crc);
    return 0;
}

/*
evaluates the send-operation. calculates transfer-speed and prints it.
*/
void evaluate(struct timeval before, struct timeval after, int msg_size, int amt, uint crc){
    time_t duration = (after.tv_usec - before.tv_usec) +(after.tv_sec - before.tv_sec)*1000000;
    long total_size = msg_size * amt * 4;
    printf("crc32-checksum: %u\n", crc);
    printf("%d packets sent\n", amt);
    if(duration != 0.0){
        double speed = 8 * total_size/duration;
        printf("%.3f mbit/s\n", speed);
    }else{
        handle_error("measured time too short. Try sending more packets");
    }
}

void handle_error(char *message){
    puts(message);
    exit(1);

}
