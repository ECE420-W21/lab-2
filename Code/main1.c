/* File:  
 *    main1.c
 *
 * Purpose:
 *    Illustrate multithreaded reads and writes to a shared array
 *    Note that it is requred to use the "getContent" and "SaveContent" function 
 #    in "common.h" to write and read the content
 *
 * Input:
 *    size of string array, server ip, and server port
 * Output:
 *    message from each thread
 *
 * Usage:    ./main1 <size_of_string_array> <server_ip> <server_port>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "timer.h"
#include "common.h"

int array_size;
char* ip_addr;
int port;  
char **theArray; 
pthread_mutex_t mutex;

void *Operate(void* rank);  /* Thread function */


/**
 * TODO:
 * Implement server functionality
*/
/*--------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
    struct sockaddr_in sock_var;
    int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
    int clientFileDescriptor;

    if (argc != 4){ 
        fprintf(stderr, "usage: %s <size_of_string_array> <server_ip> <server_port>\n", argv[0]);
        exit(0);
    }

    /* Get number of threads from command line */
    array_size = atoi(argv[1]);  
    ip_addr = argv[2];
    port = atoi(argv[3]);

    sock_var.sin_addr.s_addr=inet_addr(ip_addr);
    sock_var.sin_port=port;
    sock_var.sin_family=AF_INET;

    if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
    {   
        printf("socket has been created\n");

        // long       thread;  /* Use long in case of a 64-bit system */
        pthread_t* thread_handles; 
        int i;
        // double start, finish, elapsed;  
        
        /* Create the memory and fill in the initial values for theArray */
        theArray = (char**) malloc(array_size * sizeof(char*));
        for (i = 0; i < array_size; i ++){
            theArray[i] = (char*) malloc(COM_BUFF_SIZE * sizeof(char));
            sprintf(theArray[i], "theArray[%d]: initial value", i);
            // printf("%s\n\n", theArray[i]);
        }
    
        thread_handles = malloc (COM_NUM_REQUEST*sizeof(pthread_t)); 
        pthread_mutex_init(&mutex, NULL);
        
        listen(serverFileDescriptor,2000); 
        while(1)        //loop infinity
        {
            for(i=0;i<COM_NUM_REQUEST;i++)      
            {
                clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
                //printf("Connected to client %d\n",clientFileDescriptor);
                pthread_create(&thread_handles[i],NULL,Operate,(void *)(long)clientFileDescriptor);
            }
            for (i=0;i<COM_NUM_REQUEST;i++){
                pthread_join(thread_handles[i], NULL);
            }
            
        }
        close(serverFileDescriptor);
        pthread_mutex_destroy(&mutex);
        free(thread_handles);
        for (i=0; i<array_size; ++i){
            free(theArray[i]);
        }
        free(theArray);
        
    }else {
        printf("socket creation failed\n");
    }

    return 0;
}  /* main */


/*-------------------------------------------------------------------*/
void *Operate(void* args) {
    
    long clientFileDescriptor=(long)args;
    char buffer[COM_BUFF_SIZE]; // buffer to read client mesage
    ClientRequest* req;
    req = malloc(sizeof(ClientRequest));

    read(clientFileDescriptor,buffer,COM_BUFF_SIZE);
    ParseMsg(buffer, req);

    if(req->is_read) {
        pthread_mutex_lock(&mutex); 
        getContent(req->msg,req->pos,theArray);
        pthread_mutex_unlock(&mutex);

        write(clientFileDescriptor,req->msg, COM_BUFF_SIZE);
    }else {

        pthread_mutex_lock(&mutex); 
        setContent(req->msg,req->pos,theArray);
        getContent(req->msg,req->pos,theArray);
        pthread_mutex_unlock(&mutex);

        write(clientFileDescriptor,req->msg, COM_BUFF_SIZE);
    }

    free(req);
    close(clientFileDescriptor);
    return NULL;
}