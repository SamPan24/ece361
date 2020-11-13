//#include "packet.h"
//
//#define ERROR -1

#include <stdio.h> 
#include<unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <stdbool.h>

#include "message.h" 
#include "hash_table.h"

#define MAX 1024 
#define ERROR -1

char * authenticate(int connfd);
_Bool checkDB(Message * m);

// Driver function 
int main(int argc, char *argv[]) 
{ 
    if(argc != 2){
        printf ("Wrong input!\n");
        exit(ERROR);
    }
      
    // Making the socket
    // IPv4, TCP, Any protocol(ip?)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // Checking if socket allocation succeded
    if ( sockfd < 0 ) { 
        printf("socket creation failed"); 
        exit(ERROR); 
    } 
    
    struct sockaddr_in serverAddr, clientAddr; 
    
    // Setting all fields to empty
    memset(&serverAddr, 0, sizeof(serverAddr)); 
    memset(&clientAddr, 0, sizeof(clientAddr)); 
      
    // setting server info
    serverAddr.sin_family    = AF_INET; // IPv4 
    serverAddr.sin_port = htons(atoi(argv[1])); // port number from user
    serverAddr.sin_addr.s_addr = INADDR_ANY; 

    //bind the socket to a port and check if successful
    if ( bind(sockfd, (const struct sockaddr *)&serverAddr,  
            sizeof(serverAddr)) < 0 ) 
    { 
        printf("unable to bind "); 
        exit(ERROR); 
    }
    
    // prepare login db
    HashTable * loginDB = hash_table_init(50);
    
    // start listening
    while(true)
    {    
        // Now server is ready to listen and verification 
        if ((listen(sockfd, 5)) != 0) { 
                printf("Listen failed...\n"); 
                exit(0); 
        } 
        else
                printf("Server listening..\n"); 
        int len = sizeof(clientAddr); 

        // Accept the data packet from client and verification 
        int connfd = accept(sockfd, ( struct sockaddr *)&clientAddr, &len); 
        if (connfd < 0) { 
                printf("server acccept failed...\n"); 
                exit(0); 
        } 
        else
                printf("server acccept the client...\n"); 


        // Function for chatting between client and server 
        char * username = authenticate(connfd);
        if(username != NULL){
            // valid user
            insert_item(username, connfd, loginDB);
        } 
        else{
            printf("Failed Login attempt!\n");
        }
    }
    // After chatting close the socket 
    close(sockfd); 
} 

char * authenticate(int connfd)
{
    char * buf  = (char *)malloc(sizeof(char) * 1024); 
    
    // receive from client
    int total = 0;
//    while(1){
//        int ret = read(connfd, buf + total, 1024); 
//        total += ret;
//        if(buf[total] == '\0')
//            break;
//    }
    bzero(buf, 1024);
    total = read(connfd, buf, 1024);
    
    Message * m = string_to_packet(buf);

    if(checkDB(m)){
        // send Ack
        struct Message* ack = (Message *)malloc(sizeof(Message));
        ack->size = 0;
        ack->type = LO_ACK;
        char* sending_string = packet_to_string(ack);
        write(connfd, sending_string, sizeof(sending_string));
        free(buf);
        buf = (char *)malloc(MAX_NAME);
        strcpy(buf, m->source);
        return buf;
    }
    else{
        // send no ack
        
        struct Message* nack = (Message *)malloc(sizeof(Message));
        nack->size = 0;
        nack->type = LO_NAK;
        char* sending_string = packet_to_string(nack);
        write(connfd, sending_string, sizeof(sending_string));
        free(buf);
        return NULL;
    }
    
    free(buf);
    return NULL;
}

_Bool checkDB(Message * m){
    // Declare the file pointer 
    FILE *filePointer ; 
    filePointer = fopen("UserDB", "r") ; 
      
    if ( filePointer == NULL ) 
    { 
        printf( "No User Database found!(Not accessible)" ) ; 
        return false;
    } 
    char * username = (char *)malloc(MAX_NAME);
    char * password = (char *)malloc(MAX_NAME);
    while (true){
        
        int res = fscanf(filePointer, "%s %s\n", username, password); 
        
        
        if( res == EOF)
            break;

        if(strcmp(username, m->source) == 0){
            // User found
            printf("user found!\n");
            if(strcmp(password, m->data) == 0){
                printf("authenticated!\n");
                fclose(filePointer);
                return true;
            }
            else{
                // wrong password
                fclose(filePointer);
                return false;
            }
        }
    }
    // Not found
    fclose(filePointer) ; 
    return false;      
    
}
