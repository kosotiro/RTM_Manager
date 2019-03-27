/*************************************************************************
 * Intracom Telecom
 * Project: Control Plane Software (CPS) framework
 * Sub-project: CPS library
 * Filename: unix_domain_client.c
 * Description: UNIX DOMAIN CLIENT
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>
#include <unistd.h>
#include "msg_utilities.h"

const char *messagefromclient = "Hello World!";

int main(int argc, char *argv[]) {
   char *server_address, response[256];
   struct sockaddr_un un_addr;
   int client_socket;
   ser_buf_t *buf; 

   server_address = argv[1];

   if (server_address != NULL) {
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

      encode(&buf);

      if (send(client_socket, buf->b, buf->next, 0) > 0) {
         printf("Client is sending request towards server by using socket:%d\n", client_socket);
         dealloc_ser_buf(&buf);
      }
      else {
         perror("Problem with data sending!");
         dealloc_ser_buf(&buf);
         close(client_socket);
         exit(EXIT_FAILURE);
      }

      if (recv(client_socket, response, sizeof(response), 0) > 0) {
         printf("Data received from server...:%s\n",response);
      }
      else {
         perror("Problem with data received from server!");
         exit(EXIT_FAILURE);
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
