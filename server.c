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
#include <pthread.h>

#include "message.h" 
#include "hash_table.h"

#define MAX 1024 
#define ERROR -1

char * authenticate(UserData * d);
_Bool checkDB(Message * m);
void * newUserHandler(void * data);


//Prepare Global Login DB
HashTable * loginDB = NULL;
    

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
        printf("unable to bind!\n"); 
        exit(ERROR); 
    }
    
    loginDB = hash_table_init(50);
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
        
        UserData * data = (UserData *)malloc(sizeof(UserData));
        data->connfd = connfd;
        pthread_create( &(data->p), NULL, newUserHandler, (void *)data);
        
        
    }
    // After chatting close the socket 
    close(sockfd); 
} 


void handleUserRequests(UserData * data){
    while(1){
        char * buf = (char *)malloc(MAX);
        bzero(buf, MAX);
        int ret = read(data->connfd, buf, MAX);
        if(ret <= 0)
            continue;
        Message * m = string_to_packet(buf);
        if(m->type == MESSAGE){
            printf("User %s Sent : %s\n", m->source, m->data);
        } else if(m->type == LOGIN){
            printf("Double Login attempt!\n");
            struct Message* nack = (Message *)malloc(sizeof(Message));
            nack->size = 0;
            nack->type = LO_NAK;
            char* sending_string = packet_to_string(nack);
            write(data->connfd, sending_string, sizeof(sending_string));
            free(sending_string);
            free(nack);
        } else if(m->type == LOGOUT){
            printf("User %s Logged out.\n" , m->source);
            
            // remove from table
            remove_item(m->source, loginDB);
            
            printf("Now Logged In : \n");
            print_table(loginDB);
        }
        else {
            
            printf("Not Implemented! %d %d\n", m->type, ret);
        }
        free(buf);
    }
}


// All new users begin here
void * newUserHandler(void * data){
    UserData * d = (UserData *)data;
    char * username = authenticate(d);
    if(username != NULL){
        // valid user
        
        printf("Now Logged In : \n");
        print_table(loginDB);
        
        handleUserRequests(d);
    } 
    else{
        printf("Failed Login attempt!\n");
    }
    return data;
}

char * authenticate(UserData *d)
{
    int connfd = d->connfd;
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

    _Bool done = false;
    _Bool isValidUser = checkDB(m);
    // check if already loged in
    if(isValidUser){
        done = insert_item(m->source, d, loginDB);
        if(!done){
            printf("User already logged in!\n");
        }
    }
    if(done){
        
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
