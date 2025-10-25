//John Wright

//This program is a client for the server.c program.
//After connecting, the client can send
//messages to the server. The server
//responds, "Message received".
//The server can handle multiple client server 
//connections simultaneously.

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_LENGTH 128

int main(void){

   //Define socket name and file descriptor.
   const char sockName[] = "/tmp/socket2";
   int fd;
   
   //Define socket address and data buffers.
   struct sockaddr_un addr;
   char sendBuffer[MAX_LENGTH];
   char recBuffer[MAX_LENGTH];
   
   //Create a file descriptor for a socket.
   fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
   
   //Check for error creating socket.
   if (fd == -1) {
      perror("Socket creation error");
      return 1;
   }
   
   //Configure socket address structure.
   addr.sun_family = AF_UNIX;
   strcpy(addr.sun_path, sockName);
   
   //Connect to the server with the socket.
   if (connect(fd, (const struct sockaddr*) &addr, sizeof(struct sockaddr_un)) == -1) {
      perror("Error: Unable to connect to the server");
      fprintf(stderr, "Is the server down?\n");
      return 1;
   }
   
   //Start client loop.
   while (1) {
      //Prompt user to enter message to send.
      printf("Enter message to send: ");
      
      //Read message into buffer.
      fgets(sendBuffer, sizeof(sendBuffer), stdin);
      
      //Convert newline character into terminating zero.
      sendBuffer[strcspn(sendBuffer, "\n")] = '\0';
      
      //Write the contents of the sendBuffer array to the file descriptor of the socket.
      if (write(fd, sendBuffer, strlen(sendBuffer) + 1) == -1 ) {
         perror("Error: Failed to write");
         break;
      }
      
      //Check for message "Shutdown".
      if (strcmp(sendBuffer, "Shutdown") == 0) {
         printf("Shutting down the client.\n");
         break;
      }
      
      //Read from the server.
      if (read( fd, recBuffer, MAX_LENGTH) == -1) {
         perror("Error: Failed to read from the server");
         return 1;
      }
      
      //Print what was read from the server.
      printf("Server: %s\n", recBuffer);
   }
   
   //Close the socket.
   close(fd);
   return 0;
}
