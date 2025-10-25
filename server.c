//John Wright

//The purpose of this program is to create a server that multiple
//instances of the client.c program can connect to and
//send a message to simultaneously.

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#define MAX_LENGTH 128

const char sockName[] = "/tmp/socket2";

//Declare global variables for clients.
int clients[MAX_LENGTH];
int num_clients = 0;

//Use mutex to make sure the threads access the clients array correctly.
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

//Prototypes for cleanup and clientHandler functions.
void cleanup(int signum);
void *clientHandler(void *dataSocket);

int main(void) {

   //Return value variable for error checking.
   int ret;
   
   //variable of struct type to store address of socket.
   struct sockaddr_un addr;
   
   //variable of struct type to configure behavior of signal handling.
   struct sigaction action;
   
   //Make signal handler use cleanup function.
   action.sa_handler = cleanup;
   
   //Block all other signals while the signal handler is executing
   //to prevent interruption.
   sigfillset(&action.sa_mask);
   
   //This flag restarts system calls that are interrupted by a signal.
   action.sa_flags = SA_RESTART;
   
   //Register signal handlers.
   sigaction(SIGTERM, &action, NULL);
   sigaction(SIGINT, &action, NULL);
   sigaction(SIGQUIT, &action, NULL);
   sigaction(SIGABRT, &action, NULL);
   sigaction(SIGPIPE, &action, NULL);
     
   //Create a socket using UNIX domain.
   int connFD = socket(AF_UNIX, SOCK_SEQPACKET, 0);
   
   //Check for error.
   if (connFD == -1) {
      perror("Error: Failed to create socket");
      return 1;
   }
   
   //Configure address structure.
   addr.sun_family = AF_UNIX;
   strcpy(addr.sun_path, sockName);
   
   //Bind the socket to the address.
   if (bind(connFD, (const struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
      perror("Error: Failed to bind socket");
      return 1;
   }
   
   //Listen for connections.
   if (listen(connFD, 19) == -1) {
      perror("Listen Error");
      return 1;
   }
   
   printf("Server is listening for connections.\n");
   
   //Infinite loop to handle client connections.
   while (1) {
      //Accept connection and create data socket.
      int *dataFD = (int *) malloc(sizeof(int));
      *dataFD = accept(connFD, NULL, NULL);
      if (*dataFD == -1) {
         perror("Error accepting connection");
         return 1;
       }
       
       //Lock and unlock mutex before and after adding client to the array.
       pthread_mutex_lock(&clients_mutex);
       clients[num_clients++] = *dataFD;
       pthread_mutex_unlock(&clients_mutex);
       
       //Create new thread with default attributes for client. This thread
       //will call the clientHandler function and pass dataFD as an arguement.
       pthread_t tid;
       if (pthread_create(&tid, NULL, clientHandler, dataFD) != 0) {
          perror("Error creating thread\n");
          return 1;
       }
       
       //Detach the thread.
       pthread_detach(tid);
   }          
        
   //Close socket.
   close(connFD);
   return 0;  
}

//Cleanup function.
void cleanup(int signum) {
   printf("Quitting and cleaning up\n");
   unlink(sockName);
   exit(0);
}

//Thread function
void *clientHandler(void* dataSocket) {
   int dataFD = *((int *)dataSocket);
   char buffer[MAX_LENGTH];
   int ret;

//Infinite loop for messages from client.
while (1) {
   //read data into the buffer.
   ret = read(dataFD, buffer, MAX_LENGTH);
   
   //Check for read errors.
   if (ret == -1) {
      perror("Error reading line\n");
      cleanup(1);
   }
   else if (ret == 0) {
      printf("Client disconnected\n");
      break;
   }
   else {
      printf("Message from the client: %s\n", buffer);
      
      //Check for "Shutdown" message.
       if (strcmp(buffer, "Shutdown") == 0) {
          printf("Shutting down the server.\n");
          cleanup(0);
      }
      
       //Broadcast message to the client that sent the message.
       //Lock mutex before accessing clients array.
       pthread_mutex_lock(&clients_mutex);
       for (int i = 0; i < num_clients; ++i) { 
          if (clients[i] == dataFD) {
             write(clients[i], "Message received\n", 18);
            }
         }
         
         //Unlock mutex after accessing clients array.
         pthread_mutex_unlock(&clients_mutex);    
      }
   }
   
   //Close data socket.
   close(dataFD);

   //Terminate the thread.
   pthread_exit(NULL);
}

