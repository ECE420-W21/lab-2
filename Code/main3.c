/* File:  
 *    main3.c
 *
 * Purpose:
 *    Illustrate a multithreaded server that employs a single read/write lock to protect the 
 *    critical section 
 *
 * Input:
 *    size of string array, server ip, and server port
 *
 * Usage:    ./main3 <size_of_string_array> <server_ip> <server_port>
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
pthread_rwlock_t rwlock;

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
    double times[COM_NUM_REQUEST]; 

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
        pthread_t* thread_handles; 
        int i;  
        
        /* Create the memory and fill in the initial values for theArray */
        theArray = (char**) malloc(array_size * sizeof(char*));
        for (i = 0; i < array_size; i ++){
            theArray[i] = (char*) malloc(COM_BUFF_SIZE * sizeof(char));
            sprintf(theArray[i], "theArray[%d]: initial value", i);
        }
    
        thread_handles = malloc (COM_NUM_REQUEST*sizeof(pthread_t)); 
        pthread_rwlock_init(&rwlock, NULL);
        
        listen(serverFileDescriptor,2000); 
        while(1)        //loop infinity
        {
            for(i=0;i<COM_NUM_REQUEST;i++)  
            {
                clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
                pthread_create(&thread_handles[i],NULL,Operate,(void *)(long)clientFileDescriptor);
            }
            for (i=0;i<COM_NUM_REQUEST;i++)
            {
                pthread_join(thread_handles[i], (void **) &times[i]);
            }

            saveTimes(times, COM_NUM_REQUEST);
        }
        close(serverFileDescriptor);
        pthread_rwlock_destroy(&rwlock);
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
    double start, finish;
    double * elapsed;
    double local_elapsed = 0;
    req = malloc(sizeof(ClientRequest));

    read(clientFileDescriptor,buffer,COM_BUFF_SIZE);
    ParseMsg(buffer, req);

    GET_TIME(start);
    if(req->is_read) {
        pthread_rwlock_rdlock(&rwlock);
        getContent(req->msg,req->pos,theArray);
        pthread_rwlock_unlock(&rwlock);

        write(clientFileDescriptor,req->msg, COM_BUFF_SIZE);
    }else {
        pthread_rwlock_wrlock(&rwlock);
        setContent(req->msg,req->pos,theArray);
        pthread_rwlock_unlock(&rwlock); //Unlock writer to allow reads to continue
        
        pthread_rwlock_rdlock(&rwlock); //Read lock for return value
        getContent(req->msg,req->pos,theArray);
        pthread_rwlock_unlock(&rwlock);

        write(clientFileDescriptor,req->msg, COM_BUFF_SIZE);
    }
    GET_TIME(finish);
    local_elapsed = finish - start;
    elapsed = &local_elapsed;

    free(req);
    close(clientFileDescriptor);
    pthread_exit((void *) elapsed);
}