#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include "routeTable.h"
#include "utilities.h"

#define BUFFER_SIZE 128

int main(int argc, char *argv[]) {
   char *server_address;
   struct sockaddr_un un_addr;
   int client_socket, ready, retval;
   fd_set readfds;
   sync_msg_t msg;
   char buffer[BUFFER_SIZE];
   LinkedList routing_table;

   server_address = argv[1];

   if (server_address != NULL) {
       
      if (strlen(server_address) > (sizeof(un_addr.sun_path) - 1)) {
         fprintf(stderr, "Server socket path too long\n");
         exit(EXIT_FAILURE);
      }
       
      client_socket = socket(AF_UNIX, SOCK_STREAM, 0);

      if (client_socket == -1) {
         perror("Client Socket Creation");
         exit(EXIT_FAILURE);
      }

      memset(&un_addr, 0, sizeof(struct sockaddr_un));
      un_addr.sun_family = AF_UNIX; 
      strncpy(un_addr.sun_path, server_address, (sizeof(un_addr.sun_path) - 1) );

      if (connect(client_socket, (struct sockaddr *)&un_addr, sizeof(struct sockaddr_un)) < 0) {
         perror("Client Connection Problem");
         exit(EXIT_FAILURE);
      }
      
      initTable(&routing_table);
      FD_ZERO(&readfds);
      FD_SET(client_socket, &readfds);    
      
      while(1) {
         printf("Type 'show' to print out routing table:\n");  
         FD_SET(client_socket, &readfds);
         FD_SET(STDIN_FILENO, &readfds);
         ready = select(client_socket + 1, &readfds, NULL, NULL, NULL);
          
         if (ready == -1) {
           if (errno !=EINTR) {
             perror("Select Failure");
             cleanUpTable(&routing_table);
             close(client_socket);
             exit(EXIT_FAILURE);
           }
           else 
             continue; 
            
         }
           
         if (FD_ISSET(client_socket, &readfds)) {
           if (recv(client_socket, &msg, sizeof(sync_msg_t), 0) > 0)
             TableAction(NULL, &routing_table, &msg);
           else {
             perror("Problem with data received from server!");
             cleanUpTable(&routing_table);
             close(client_socket);
             exit(EXIT_FAILURE);
           }   
         }
           
         if (FD_ISSET(STDIN_FILENO, &readfds)) {
           retval = read(STDIN_FILENO, buffer, BUFFER_SIZE);
               
           if (retval > 0) {
             /* Remove trailing newline character from the input buffer if needed. */
             buffer[retval-1] = '\0';
             
             if (!strcmp(strlwr(buffer), "show")) {
               printAllentries(&routing_table);  
             }
             else {
               fprintf(stderr, "Unknown option\n");
               continue;
             }
           }
         }
       }

      if (close(client_socket) == -1) {
         perror("Client Socket Close Error!");
         exit(EXIT_FAILURE);
      }
    }
    else {
       printf("Wrong usage!\n");
       printf("Right Usage: ./unix_domain_client \"socket_path\"\n");
    }    

    exit(EXIT_SUCCESS);
}
