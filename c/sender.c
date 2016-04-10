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

int main(int argc, char *argv[]){

    
    //handle wrong number of cmd args
    if(argc != 5){
        handleError("Usage: ./sender <receiver name> <port> <number of packets to send> <message length>");
    }

    //declare required local vars
    int sock, amt_sent, sent;
    struct sockaddr_in receiver;
    struct hostent *host;
    struct timeval before, after;
    int msgLength;
    msgLength = atoi(argv[4]);
    char buffer[msgLength + 6];
    char* msg;
    msg = createMsg(msgLength);
    int amt_send;
    amt_send = atoi(argv[3]);

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
    for(amt_sent = 0; amt_sent < amt_send; amt_sent++){
        //zero out buffer
        bzero(buffer, msgLength + 6);
        //set message to be sent
        sprintf(buffer, "%d %s", amt_sent,msg);
        //send message in buffer as datagram and handle error
        sent = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&receiver, sizeof(struct sockaddr_in));
        if(sent == 0){
            handleError("package not sent successfully");
        }
    }
    gettimeofday(&after, NULL);
    evaluate(before,after,strlen(buffer),amt_send);
    free(msg);
    return 0;
}

void evaluate(struct timeval before, struct timeval after, int msg_size, int amt){
    time_t duration = (after.tv_usec - before.tv_usec)/1000 +(after.tv_sec - before.tv_sec)*1000;
    int total_size = msg_size * amt;
    double speed = total_size/duration;
    printf("%d packets received\n", amt);
    printf("%.2f KB/s\n", speed);
}
    

char* createMsg(int length){
    char *str = malloc(length);
    memset(str, 'a', length-1);
    str[length-1] = '\0';
    return str;
    
} 

/*
handleError takes a error message as pointer to a char sequence prints it and exits the application
*/
void handleError(char *message){
    puts(message);
    exit(1);

}