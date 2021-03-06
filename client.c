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
#include <pthread.h>
#include <signal.h>

#include "message.h"
#include "hash_table.h"

#define SIG_ABRT -10

typedef enum Command {
    Login, 
    Logout,
    Join,
    Leave,
    Create,
    List,
    Quit
} Command;

MessageList * controlList = NULL;
MessageList * userMessages= NULL;

_Bool forceLogout = false;

pthread_mutex_t controlMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t userMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t conrolCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t userCond = PTHREAD_COND_INITIALIZER;

void addToMsgList(Message *m , MessageList *l){
    
    while(l->next != NULL){
        l = l->next;
    }
    l->next = (MessageList *)malloc(sizeof(MessageList));
    l->next->m = m;
    l->next->next = NULL;
}

Message * popControlList(){
    
    pthread_mutex_lock(&controlMutex);
    while(controlList->next == NULL){
        // Block
        pthread_cond_wait(&conrolCond, &controlMutex);
    }
    
    Message * res = controlList->next->m;
    
    controlList->next = controlList->next->next;
    
    pthread_mutex_unlock(&controlMutex);

    return res;
}
Message * popUserList(){
    
    pthread_mutex_lock(&userMutex);
    while(controlList->next == NULL){
        // Block
        pthread_cond_wait(&userCond, &userMutex);
    }
    
    Message * res = controlList->next->m;
    
    controlList->next = controlList->next->next;
    
    pthread_mutex_unlock(&userMutex);

    return res;
}

void freeMsgList(MessageList *l){
    MessageList * head = l->next;
    while(head != NULL){
        MessageList * temp = head->next;
        if(head->m->type == MESSAGE){
            //printf("Backlog Message| From: %8s Data: %s\n", head->m->source, head->m->data);
        }
        free(head->m);
        free(head);
        head = temp;
    }
}

void message_receiver_terminate_handler(int sig){
    // free list
    pthread_mutex_lock(&userMutex);
    freeMsgList(userMessages);
    pthread_mutex_unlock(&userMutex);

    pthread_mutex_lock(&controlMutex);
    freeMsgList(controlList);
    pthread_mutex_unlock(&controlMutex);

    // reset
    controlList->next = NULL;
    userMessages->next = NULL;
}

void * message_receiver(void * sockfd_ptr){

    // attach a sig handler
    signal(SIG_ABRT, message_receiver_terminate_handler);

    int sockfd = *(int *)sockfd_ptr;
    char buff[MAX];
    
    while(1){
        bzero(buff, MAX); 
        int ret = read(sockfd, buff, MAX);
        if(ret <= 0)
            continue;
        struct Message* received = string_to_packet(buff);
        if (received->type == MESSAGE) { 
            pthread_mutex_lock(&userMutex);
            
            addToMsgList(received, userMessages);
            
            pthread_cond_signal(&userCond);
            
            printf("%s: %s\n", received->source, received->data );
            
            pthread_mutex_unlock(&userMutex);
        }else{
            
            pthread_mutex_lock(&controlMutex);
            if(received->type == EXIT){
                // final message, server exitted
                printf("Server Exited!\n");
                forceLogout = true;
            }
            
            addToMsgList(received, controlList);
            
            pthread_cond_signal(&conrolCond);
            
            pthread_mutex_unlock(&controlMutex);
        }
    }
    return sockfd_ptr;
}

int main() 
{
    forceLogout = false;
    // base
    controlList = (MessageList *)malloc(sizeof(MessageList));
    userMessages = (MessageList *)malloc(sizeof(MessageList));
    
    // init
    controlList->m = NULL;
    controlList->next = NULL;
    userMessages->m = NULL;
    userMessages->next = NULL;
    
    int sockfd, connfd; 
    struct sockaddr_in servaddr; 

    pthread_t receiver_thread;

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
        if(forceLogout && loged_in){
            loged_in = false;
            pthread_kill(receiver_thread, SIG_ABRT);
            close(sockfd);
            printf("Forced Logout!\n");
            continue;
        }
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
                        
                        if(!loged_in){
                            printf("Not Logged in.\n");
                            break;
                        }
                        // handle
                        loged_in = false;
                        
                        struct Message* sending = (Message *)malloc(sizeof(Message));
                        sending->size = 0;
                        sending->type = LOGOUT;
                        strcpy (sending->source, username);
                        strcpy(sending->data, "");
                        char* sending_string = packet_to_string(sending);
                        write(sockfd, sending_string, strlen(sending_string)); 
                        
                        pthread_kill(receiver_thread, SIG_ABRT);
                        close(sockfd);
                        // next command
                        break;
                    } 
                    else if(strcmp(word , "/joinsession") == 0){
                        c = Join;
                        
                        if(!loged_in){
                            printf("Not Logged in.\n");
                            break;
                        }
                        // handle later
                    } 
                    else if(strcmp(word , "/leavesession\n") == 0){
                        c = Leave;
                        
                        if(!loged_in){
                            printf("Not Logged in.\n");
                            break;
                        }
                        // send packets
                        struct Message* sending = (Message *)malloc(sizeof(Message));
                        sending->size = 0;
                        sending->type = LEAVE_SESS;
                        strcpy(sending->data, "");
                        strcpy(sending->source, username);
                                
                        char* sending_string = packet_to_string(sending);
                        
                        write(sockfd, sending_string, strlen(sending_string));
                        // handle
                        printf("left current session\n");
                        // next command
                        break;
                    } 
                    else if(strcmp(word , "/createsession") == 0){
                        c = Create;
                        
                        if(!loged_in){
                            printf("Not Logged in.\n");
                            break;
                        }
                        // handle later
                    } 
                    else if(strcmp(word , "/list\n") == 0){
                        c = List;
                        
                        if(!loged_in){
                            printf("Not Logged in.\n");
                            break;
                        }

                        struct Message* sending = (Message *)malloc(sizeof(Message));

                        sending->size = 0;
                        strcpy(sending->data, "");
                        strcpy(sending->source, username);
                        sending->type = QUERY;
                        char* sending_string = packet_to_string(sending);
                        write(sockfd, sending_string, strlen(sending_string));
                        
//                        char buff[MAX];
//                        bzero(buff, sizeof(buff)); 
//                        read(sockfd, buff, sizeof(buff));
//
//                        struct Message* received = string_to_packet(buff);
                        Message * received = popControlList();
                        if (received->type == QU_ACK) { 
                            printf("Currently Active Sessions:\n%s", received->data);
                        } 
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
                        printf("Invalid Command!\n");
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
                        
                        Message * received = popControlList();
                        if (received->type == JN_ACK) {
                            printf ("Joined session: %s\n", word);
                        } 
                        else if (received->type == JN_NAK) {
                            if(strcmp(received->data, "leave_first") == 0)
                                printf ("Join failed: Leave current session first\n");
                            else
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

         
                            Message * received = popControlList();
                            if (received->type == NS_ACK) { 
                                printf ("new session created\n");
                                printf ("session ID: %s\n", word);
                            } 
                            else if (received->type == NS_NACK) { 
                                printf ("session already exists\n");
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
            if(loged_in){
                printf("Logout first!\n");
                continue;
            }
                
            if(word_count != 5){
                printf("Invalid Arguments!\n");
                continue;
            }
            // socket create and varification 
            sockfd = socket(AF_INET, SOCK_STREAM, 0); 
            if (sockfd == -1) { 
                    printf("Socket Creation Failed!\n"); 
                    continue;
            } 
            else
                    printf("Socket successfully created\n"); 

            pthread_create(&receiver_thread, NULL, message_receiver, &sockfd);
            bzero(&servaddr, sizeof(servaddr)); 

            // assign IP, PORT 
            servaddr.sin_family = AF_INET; 
            servaddr.sin_addr.s_addr = inet_addr(login_args[2]); 
            servaddr.sin_port = htons(atoi(login_args[3])); 

            // connect the client socket to server socket 
            if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) { 
                printf("Server Connection Failed!\n"); 
                close(sockfd);

                pthread_kill(receiver_thread, SIG_ABRT);
                continue;
            } 
            else{
                forceLogout = false;
                printf("Connected!\n"); 
            }
            // send/receive packets
            struct Message* sending = (Message *)malloc(sizeof(Message));

            strcpy (sending->data, login_args[1]);
            strcpy (sending->source, login_args[0]);
            
            sending->size = strlen(sending->data);
            sending->type = LOGIN;
            char* sending_string = packet_to_string(sending);
            
            write(sockfd, sending_string, strlen(sending_string));
            
            // wait for lo_ack or lo_nack

            Message * received;
            do{
                received = popControlList();
            }while(received->type == EXIT);
            
            if (received->type == LO_ACK) { 
                loged_in = true; 
                username = login_args[0];
                
                // Dont delete login_args 0
                continue; 
            } 
            else if(received->type == LO_NAK){
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




