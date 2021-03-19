/*
 Student ID: 1607869
 Name: Zhihao Lin

 Assignment 2 - server source code
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

//---- primitives ----
#define START_ID 0xFFFF
#define END_ID 0xFFFF
#define CLIENT_ID 0xFF
#define Acc_Per 0xFFF8
#define Not_Paid 0xFFF9
#define Not_Exist 0xFFFA
#define Access_OK 0xFFFB
#define LENGTH 0xFF
#define _2G 02
#define _3G 03
#define _4G 04
#define _5G 05

//modify the following value to other port (up to 65535)
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

//---- format of linked list that stored information from database ----
typedef struct DATABASE{
    unsigned int number;
    unsigned char techonogy;
    unsigned char paid;
    struct DATABASE *next;
} DATABASE;


//---- this function used for storing contents from database file to server ----
struct DATABASE *head = NULL;
void get_info_from_database() {
    unsigned short technology;
    unsigned short paid;
    unsigned int subscriber_no;
    FILE *file;

    file = fopen("Verification_Database.txt", "r"); //open the database file
    if(file){
        while((fscanf(file, "%u %hi %hi", &subscriber_no, &technology, &paid) != EOF)){
            struct DATABASE *temp = (struct DATABASE*) malloc(sizeof(struct DATABASE)); //create a link
            temp->number = subscriber_no;
            temp->techonogy = technology;
            temp->paid = paid;

            //insert each data to first position of the linked list
            temp->next = head;
            head = temp;
        }
        fclose(file);
    } else{ //open_file_error
        perror("open_file_Error: ");
        exit(1);
    }
}


//---- this function is to find the information of the subscriber ----
DATABASE* find(unsigned int number){
    struct DATABASE* curr = head;
    while(curr->number != number){
        if(curr->next == NULL) return NULL; // number doesn't exist
        else curr = curr->next;
    }
    return curr; // found the number in database
}


int main(int argc, char *argv[]){

    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof client_addr;
    int socketfd;
    DATA_PACKET client_msg, server_msg;


    //--- call this function to store infomation from database file to server ---
    get_info_from_database();

    //------ Create UDP socket ------
    if((socketfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket error: ");
        exit(1);
    }

    //------ Set IP and port------
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);
    
    //------ Bind socket to host ------
    if(bind(socketfd, (struct sockaddr *)&server_addr, sizeof server_addr) < 0){
        perror("binding error: ");
        exit(1);
    }
    printf("Listening for client messages...\n\n");

    
    while(1){
        //------ Receiving client message ------
        if (recvfrom(socketfd, &client_msg, sizeof(DATA_PACKET), 0,
             (struct sockaddr*)&client_addr, &addr_len) < 0){
            perror("message_receive_error: ");
            exit(1);
        } else{
            // msg received, setup response message
            server_msg.start_id = START_ID;
            server_msg.client_id = client_msg.client_id;
            server_msg.segno = client_msg.segno;
            server_msg.len = client_msg.len;
            server_msg.technology = client_msg.technology;
            server_msg.subscriber_no = client_msg.subscriber_no;
            server_msg.end_id = END_ID;
        }

        //------ Verifying subsribers' information ------
        DATABASE *verification = find(client_msg.subscriber_no);
        
        //----- check verification result -----
        if(verification == NULL){ //number doesn't exist
            server_msg.type = Not_Exist;
            printf("packet %d - Subscriber number:%u  is not found\n", client_msg.segno, client_msg.subscriber_no);
        }
        else if(verification->techonogy != client_msg.technology){ //technology doesn't match
            server_msg.technology = verification->techonogy; //send the correct technology
            server_msg.type = Not_Exist;
            printf("packet %d - Subscriber number:%u is found, but the technology does not match\n", client_msg.segno, client_msg.subscriber_no);
        }
        else if(verification->paid == 0){ //Subscriber has not paid
            server_msg.type = Not_Paid;
            printf("packet %d - Subscriber number:%u has not paid\n", client_msg.segno, client_msg.subscriber_no);
        }
        else{ //no error, verified subscriber
            server_msg.type = Access_OK;
            printf("packet %d - Subscriber number:%u permitted to access the network\n", client_msg.segno, client_msg.subscriber_no);
        }

        //------ send response message to client -------
        if (sendto(socketfd, &server_msg, sizeof(DATA_PACKET), 0,
             (struct sockaddr*)&client_addr, addr_len) < 0){
            perror("send_error: ");
            exit(1);
        }
    }
    /* To keep server running, close() will not be used.
     you may need to manually stop the program */
    return 0;
}
