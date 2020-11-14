#include <stdbool.h>
#include <netdb.h> 
#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <assert.h>

#include "message.h"

#define MAX 80

typedef enum Command {
    Login, 
    Logout,
    Join,
    Leave,
    Create,
    List,
    Quit
} Command;
//
//void func(int sockfd) 
//{ 
//    char buff[MAX]; 
//    int n; 
//    for (;;) { 
//            bzero(buff, sizeof(buff)); 
//            printf("Enter the string : "); 
//            n = 0; 
//            while ((buff[n++] = getchar()) != '\n') 
//                    ; 
//            write(sockfd, buff, sizeof(buff)); 
//            bzero(buff, sizeof(buff)); 
//            read(sockfd, buff, sizeof(buff)); 
//            printf("From Server : %s", buff); 
//            if ((strncmp(buff, "exit", 4)) == 0) { 
//                    printf("Client Exit...\n"); 
//                    break; 
//            } 
//    } 
//} 

int main() 
{
    int sockfd, connfd; 
    struct sockaddr_in servaddr, cli; 

    // socket create and varification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
            printf("socket creation failed...\n"); 
            exit(0); 
    } 
    else
            printf("Socket successfully created..\n"); 

    
    _Bool quit = false;
    _Bool loged_in = false;
    char * username = NULL;
    while(!quit){
        Command c;
        char * command = (char *)malloc(50);
        char  * login_args [4];
        size_t len = 50;
        printf(">");
        getline(&command, &len, stdin);

        char * word_array= (char *)malloc(len +1);
        strncpy(word_array , command, len);
        char * word = strtok(word_array, " ");

        int word_count = 0;
        _Bool text = false; // Assume Command
        while (word != NULL)
        {
            if(strcmp(word, "") == 0){
                word = strtok (NULL, " ");
                continue;
            }
            // Handle word
            if(word_count == 0){ 
                if(word[0] == '/'){
                    // Is a command
                    if(strcmp(word , "/login") == 0){
                        c = Login;

                        // handle later
                    }
                    else if(strcmp(word , "/logout\n") == 0){
                        c = Logout;

                        // handle
                        loged_in = false;
                        
                        struct Message* sending = (Message *)malloc(sizeof(Message));
                        strcpy (sending->source, username);
                        sending->size = 0;

                        sending->type = LOGOUT;
                        char* sending_string = packet_to_string(sending);
                        write(sockfd, sending_string, strlen(sending_string)); 
                        // next command
                        break;
                    } 
                    else if(strcmp(word , "/joinsession") == 0){
                        c = Join;
                        
                        // handle later
                    } 
                    else if(strcmp(word , "/leavesession\n") == 0){
                        c = Leave;
                        // send packets
                        struct Message* sending = (Message *)malloc(sizeof(Message));
                        sending->size = 0;
                        sending->type = LEAVE_SESS;
                        
                        char* sending_string = packet_to_string(sending);
                        
                        write(sockfd, sending_string, strlen(sending_string));
                        // handle
                        printf("left current session\n");
                        // next command
                        break;
                    } 
                    else if(strcmp(word , "/createsession") == 0){
                        c = Create;
                        
                        // handle later
                    } 
                    else if(strcmp(word , "/list\n") == 0){
                        c = List;

                        struct Message* sending = (Message *)malloc(sizeof(Message));

                        sending->size = 0;

                        sending->type = QUERY;
                        char* sending_string = packet_to_string(sending);
                        write(sockfd, sending_string, strlen(sending_string));
                        
                        char buff[MAX];
                        bzero(buff, sizeof(buff)); 
                        read(sockfd, buff, sizeof(buff));

                        struct Message* received = string_to_packet(buff);
                        if (received->type == QU_ACK) { 
                            printf("Currently Active Sessions:\n%s", received->data);
//                            //print out the list of connections
//                            char *list;
//                            strcpy(list, received->data); 
//                            
//                            char* token = strtok(list, " "); 
//                        
//                            // Keep printing tokens while one of the 
//                            // delimiters present in str[]. 
//                            int check = 0;
//                            while (token != NULL) { 
//                                if (check%2 == 0){
//                                    printf("user:   %s\n", token); 
//                                    check++;
//                                    token = strtok(NULL, " ");
//                                }
//                                else{
//                                    printf("session:   %s\n", token); 
//                                    check++;
//                                    token = strtok(NULL, " ");
//                                }
//                            }
                        } 

                        

                        // next command
                        break;
                    } 
                    else if(strcmp(word , "/quit\n") == 0){
                        c = Quit;

                        // handle
                        if (loged_in == true){
                            struct Message* sending = (Message *)malloc(sizeof(Message));
                            strcpy (sending->source, username);
                            sending->size = 0;

                            sending->type = LOGOUT;
                            char* sending_string = packet_to_string(sending);
                            write(sockfd, sending_string, strlen(sending_string)); 
                            loged_in = false;
                        }

                        quit = true;
                        
                        // next command
                        break;
                    } 
                    else {
                        printf("Invalid Command! WORD: %s\n", word);
                    }
                }
                else {
                    // is text
                    text = true;
                    break;
                }
            }
            else{
                // Not first word
                if(text){
                    // 
                    printf("ERROR!");
                    exit(0);
                }
                else{
                    // handle command argument
                    if(c == Login){
                        if(word_count <= 5){
                            login_args[word_count -1] = (char *)malloc(50);
                            strcpy(login_args[word_count -1] , word);
                        }else{
                            printf("Invalid Arguments\n");
                        }
                    }
                    else if (c == Join){
                        if(word_count == 1){
                            // Session ID 
                            
                            // join with sess id word
                            // send/receive packets
                            
                        struct Message* sending = (Message *)malloc(sizeof(Message));

                        strcpy(sending->source, username);
                        strcpy (sending->data, word);
                        sending->data[strlen(word) -1] = '\0';
                        
                        sending->size = strlen(sending->data);
                        sending->type = JOIN;
                        char* sending_string = packet_to_string(sending);
                        
                        write(sockfd, sending_string, strlen(sending_string));
                        
                        // wait for lo_ack or lo_nack
                        char buff[MAX];
                        bzero(buff, sizeof(buff)); 
                        read(sockfd, buff, sizeof(buff));

                        struct Message* received = string_to_packet(buff);
                        if (received->type == JN_ACK) {
                            printf ("Joined session: %s\n", word);
                        } 
                        else if (received->type == JN_NAK) {
                            printf ("Join failed: session %s does not exist\n", word);
                        }
                        }
                        else {
                            printf("Invalid Arguments\n");
                        }
                    }
                    else if (c == Create) {
                        
                        if(word_count == 1){
                            // Session ID
                            // send/receive packets
                            struct Message* sending = (Message *)malloc(sizeof(Message));
                            
                            strcpy(sending->source, username);
                            strcpy (sending->data, word);
                            sending->data[strlen(word) -1] = '\0';
                            sending->size = strlen(sending->data);
                            sending->type = NEW_SESS;
                            char* sending_string = packet_to_string(sending);

                            write(sockfd, sending_string, strlen(sending_string));

                            // wait for lo_ack or lo_nack
                            char buff[MAX];
                            bzero(buff, sizeof(buff)); 
                            read(sockfd, buff, sizeof(buff));

                            struct Message* received = string_to_packet(buff);
                            if (received->type == NS_ACK) { 
                                printf ("new session created\n");
                                printf ("session ID: %s\n", word);
                            } 
                            // create with sess id word
                        }
                        else {
                            printf("Invalid Arguments\n");
                        }
                    }
                    else {
                        printf("Invalid Arguments\n");
                    }
                }
            }

            // get next word
            word_count++;
            word = strtok (NULL, " ");
        }
        
        //  Only Login
        
        if (text){
            if(loged_in){
                struct Message* sending = (Message *)malloc(sizeof(Message));
                strncpy (sending->data, command, strlen(command) - 1);
                strcpy (sending->source, username);
                sending->size = strlen(sending->data);
                
                sending->type = MESSAGE;
                char* sending_string = packet_to_string(sending);
                write(sockfd, sending_string, strlen(sending_string)); 
            }else {
                printf("Login first!\n");
                break;
            }
        }
        else if(c == Login){
            assert(word_count == 5);
            bzero(&servaddr, sizeof(servaddr)); 

            // assign IP, PORT 
            servaddr.sin_family = AF_INET; 
            servaddr.sin_addr.s_addr = inet_addr(login_args[2]); 
            servaddr.sin_port = htons(atoi(login_args[3])); 

            // connect the client socket to server socket 
            if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) { 
                    printf("connection with the server failed...\n"); 
                    exit(0); 
            } 
            else
                    printf("connected to the server..\n"); 
            
            // send/receive packets
            struct Message* sending = (Message *)malloc(sizeof(Message));

            strcpy (sending->data, login_args[1]);
            strcpy (sending->source, login_args[0]);
            
            sending->size = strlen(sending->data);
            sending->type = LOGIN;
            char* sending_string = packet_to_string(sending);
            
            write(sockfd, sending_string, strlen(sending_string));
            
            // wait for lo_ack or lo_nack
            char buff[MAX];
            bzero(buff, sizeof(buff)); 
            read(sockfd, buff, sizeof(buff));

            struct Message* received = string_to_packet(buff);
            if (received->type == LO_ACK) { 
                loged_in = true; 
                username = login_args[0];
                // Dont delete login_args 0
                continue; 
            } 
            else {
                loged_in = false;
                printf ("login failed, try again\n");
                continue;
            }
        } 
        // command done
        free(command);
        free(word_array);
    }

    // close the socket 
    close(sockfd); 
} 


