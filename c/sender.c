/*
SENDER 
Please read "Dokumentation_und_Auswertung.pdf" for more precise information
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
#include <limits.h>
#include "sender.h"

int main(int argc, char *argv[]){

    
    //handle wrong number of cmd args
    if(argc != 5){
        handle_error("Usage: ./sender <receiver name> <port> <number of packets to send> <packet payload size>");
    }

    //declare and define required local vars
    int sock, amt_send, amt_sent, sent, total_pack_size, payload, sockaddr_len, i;
    struct sockaddr_in receiver;
    struct hostent *host;
    struct timeval before, after;
    uint crc;
    crc = 0;
    payload = atoi(argv[4]);
    total_pack_size = payload + 1;
    int buffer[total_pack_size];
    for(i = 1; i < total_pack_size; i++){
        buffer[i] = 9;
    }
    amt_send = atoi(argv[3]);

    //create udp ipv4 socket and handle creation error
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0){
        handle_error("socket could not be created");
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
        buffer[0] = amt_sent;
        //check cases
        if(amt_sent < amt_send - 1){
            crc = crc32(crc, (const void*)buffer, total_pack_size);
        }else{
            crc = crc32(crc, (const void*)buffer, total_pack_size - 1);
            buffer[total_pack_size - 1] = crc;
        }
        //send message in buffer as datagram and handle error
        sent = sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&receiver, sizeof(struct sockaddr_in));
        if(sent == 0){
            handle_error("package not sent successfully");
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
    time_t duration = (after.tv_usec - before.tv_usec)/1000 +(after.tv_sec - before.tv_sec)*1000;
    int total_size = msg_size * amt * 4;
    printf("crc32-checksum: %u\n", crc);
    printf("%d packets sent\n", amt);
    if(duration != 0.0){
        double speed = total_size/duration;
        printf("%.2f KB/s\n", speed);
    }else{
        handle_error("measured time too short. Try sending more packets");
    }
}

void handle_error(char *message){
    puts(message);
    exit(1);

}
