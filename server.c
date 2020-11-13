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
#define MAX 80 
#define ERROR -1
// Function designed for chat between client and server. 
void func(int sockfd) 
{ 
	char buff[MAX]; 
	int n; 
	// infinite loop for chat 
	for (;;) { 
		bzero(buff, MAX); 

		// read the message from client and copy it in buffer 
		read(sockfd, buff, sizeof(buff)); 
		// print buffer which contains the client contents 
		printf("From client: %s\t To client : ", buff); 
		bzero(buff, MAX); 
		n = 0; 
		// copy server message in the buffer 
		while ((buff[n++] = getchar()) != '\n') 
			; 

		// and send that buffer to client 
		write(sockfd, buff, sizeof(buff)); 

		// if msg contains "Exit" then server exit and chat ended. 
		if (strncmp("exit", buff, 4) == 0) { 
			printf("Server Exit...\n"); 
			break; 
		} 
	} 
} 

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
    func(connfd); 

    // After chatting close the socket 
    close(sockfd); 
} 




