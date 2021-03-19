/*
 Student ID: 1607869
 Name: Zhihao Lin

 Assignment 2 - client source code
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

//---- primitives ----
#define START_ID 0xFFFF
#define END_ID 0xFFFF
#define CLIENT_ID 0xFF
#define Acc_Per 0xFFF8
#define Not_Paid 0xFFF9
#define Not_Exist 0xFFFA
#define Access_OK 0xFFFB
#define LENGTH 0xFF
#define Subscriber_Number 0xFFFFFFFF //maximum Decimal phone number 4294967295
#define _2G 02
#define _3G 03
#define _4G 04
#define _5G 05

//change following value to set 5 consecutive phone numbers (starting from this number)
//up to 4294967291
#define number 3000000000

//modify the following value to other port (up to  65535)
#define PORT 2000

//---- Packet format for all types of packet (receive and response) ----
typedef struct DATA_PACKET{
    unsigned short start_id;
    unsigned char client_id;
    unsigned short type;
    unsigned char segno;
    unsigned char len;
    unsigned char technology;
    unsigned int subscriber_no;
    unsigned short end_id;
} DATA_PACKET;

int main(int argc, char *argv[]){
    
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof server_addr;
    DATA_PACKET server_msg, packet_set[5];
    int socketfd;


    //---- Create UDP socket ----
    if ((socketfd=socket(PF_INET, SOCK_DGRAM, 0)) < 0){
        printf("socket error\n");
        exit(1);
    }

    //---- Set IP and port ----
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);

    
    //----- manually set technology fields of each packet -----
    packet_set[0].technology = _2G; //simulate not paid
    packet_set[1].technology = _3G; //simulate technology doesn't match
    packet_set[2].technology = _4G; //simulate number doesn't found
    packet_set[3].technology = _5G; //simulate permitted number
    packet_set[4].technology = _5G; //simulate permitted number

    //----- set rest of the fields of each packet -----
    for(int i = 0; i < 5; i++){
        packet_set[i].start_id = START_ID;
        packet_set[i].client_id= CLIENT_ID;
        packet_set[i].type = Acc_Per;
        packet_set[i].segno = i;
        //set consecutive phone numbers (range from 3000000000 to 3000000004)
        packet_set[i].subscriber_no = number + i;
        packet_set[i].len = sizeof packet_set[i].technology + sizeof packet_set[i].subscriber_no;
        packet_set[i].end_id = END_ID;
    }
    
    //----- setup ack_timer -----
    struct pollfd pfds[0];
    pfds[0].fd = socketfd;
    pfds[0].events = POLLIN;
    

    int curr = 0;
    while(curr < 5) { //loop to send all packets of a set
        //------ start sending packets ------
        printf("Client: Sending packet %d\n", packet_set[curr].segno);
        if(sendto(socketfd, &packet_set[curr], sizeof(DATA_PACKET), 0,
                  (struct sockaddr*)&server_addr, addr_len) < 0) {
            perror("send_error: ");
            exit(1);
        }
        
        //------ activate ack_timer and wait for event ------
        int event = poll(&pfds[0], 1, 3000), retry_counter = 1;
        while(event == 0){ //if no response
            if(retry_counter <= 3){ //reset ack_timer up to 3 times
            printf("Client: resending packet %d. attempt no.%d / 3\n", packet_set[curr].segno, retry_counter);
            if(sendto(socketfd, &packet_set[curr], sizeof(DATA_PACKET), 0,
                      (struct sockaddr *)&server_addr, addr_len) < 0) {
                perror("send_error: ");
                exit(1);
            }
            retry_counter++;
            event = poll(&pfds[0], 1, 3000);
          }
          else{ //resend reach 3 times, server no response
            printf("Server does not respond\n");
            close(socketfd);
            exit(0);
          }
        }
        
        
        //------ check for the event result of timer ------
        if(event < 0){ //polling error
            perror("poll_error: ");
            exit(1);
        }
        else { //server responsed
            if(recvfrom(socketfd, &server_msg, sizeof(DATA_PACKET), 0,
                        (struct sockaddr *)&server_addr, &addr_len) < 0){
                perror("message_receive_error: ");
                exit(1);
            }
            
            //------- check response type and print result ---------
            //Subscriber has not paid message
            if(server_msg.type == Not_Paid){
                printf("Server: Subscriber number - %u has not paid.\n\n", server_msg.subscriber_no);
            }
            
            //Subscriber does not exist on database message
            else if(server_msg.type == Not_Exist){
                if(server_msg.technology != packet_set[curr].technology) //case2
                    printf("Server: Subscriber number - %u is found, but the technology does not match.\n\n", server_msg.subscriber_no);
                else //case1
                    printf("Server: Subscriber number - %u  is not found.\n\n", server_msg.subscriber_no);
            }
            
            //Subscriber permitted to access the network message
            else if(server_msg.type == Access_OK){
                printf("Server: Subscriber number - %u permitted to access the network\n\n", server_msg.subscriber_no);
            }
        }
        curr++;
    }
    close(socketfd);
}
