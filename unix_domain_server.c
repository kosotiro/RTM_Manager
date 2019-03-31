/*************************************************************************
 * Intracom Telecom
 * Project: Control Plane Software (CPS) framework
 * Sub-project: CPS library
 * Filename: unix_domain_server.c
 * Description: UNIX Domain Server
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include "routeTable.h"
#include "utilities.h"


#define TIME_INTERVAL 120
#define PEND_CON 20 /* Indicates the number of pending connections for listen function. */
#define MAX_CLIENTS_SUPPORTED 32
#define BUFFER_SIZE 128

static volatile sig_atomic_t keeprunning = 1; /* controlling server process state (running / terminated) */
int client_handles[MAX_CLIENTS_SUPPORTED];
int client_aligned[MAX_CLIENTS_SUPPORTED];

void sig_handler(int sig) {
    if (sig == SIGINT) {
      keeprunning = 0; /* stop the server process currently running */
      fprintf(stderr, "Stopping server process!\n");
    }
}


/*Remove all the FDs, if any, from the the array*/
static void intit_client_handler_set() {

    int i;
    
    for(i=0; i < MAX_CLIENTS_SUPPORTED; i++) {
      client_handles[i] = -1;
      client_aligned[i] = false;
    }
}

/*Add a new FD to the monitored_fd_set array*/
static void add_to_client_handler_set(int skt_fd) {

    int i;
    
    for (i=0; i < MAX_CLIENTS_SUPPORTED; i++) {
      if (client_handles[i] != -1)
        continue;     
      client_handles[i] = skt_fd;
      break;
    }
}


static void remove_from_client_handler_set(int skt_fd) {

    int i;
    
    for (i=0; i < MAX_CLIENTS_SUPPORTED; i++) {
      if (client_handles[i] != skt_fd)
        continue;
      client_handles[i] = -1;
      break;
    }
}


static void alignClients(LinkedList *route_table, sync_msg_t *msg) {
  
  int client, entryidx;
  data *tableEntry;
  sync_msg_t message;
  /*****************************************************
   * send route table entries to all connected clients * 
   * when a CIR is succesfully established             *
   * ***************************************************/
  
  for (client = 0; client < MAX_CLIENTS_SUPPORTED; client ++) {
    if (client_handles[client] != -1) { /* valid client */
      if (route_table != NULL && !client_aligned[client]) {
        printf("Iam here\n");
        printf("listNodeNum:%d\n", listNodeNum(&route_table));
        for (entryidx = 1; entryidx <= listNodeNum(&route_table); entryidx++) {
          printf("mphka");
          tableEntry = getEntry(&route_table, entryidx);
          if (tableEntry != NULL) {
            message.op_code = CREATE;
            memcpy(&message.msg_body, tableEntry, sizeof(data));
            if (send(client_handles[client], &message, sizeof(sync_msg_t), 0) > 0)
              printf("Update client:%d\n", client_handles[client]);
            else
              perror("Send Problem");
          }
        }
        client_aligned[client] = true;
      }
      else {
        printf("print:%s %d %s %s\n", msg->msg_body.destination, msg->msg_body.mask, msg->msg_body.gateway_ip, msg->msg_body.oif);
        if (send(client_handles[client], msg, sizeof(sync_msg_t), 0) > 0)
          printf("Update client:%d\n", client_handles[client]);
        else 
          perror("Send Problem");
      }
    }
  }
}


int main(int argc, char *argv[]) {
   /* structure for providing timeout on select */
   struct timeval timeout;
   struct sigaction sa;
   int i, maxfd, fd, client_socket, ready, server_socket;
   unsigned int len;
   int flags, retval, fdt, bytes, random_pos;
   bool close_con = false;
   char request[256], ch, option[2];
   fd_set workingset;
   /* UNIX domain socket structure */
   struct sockaddr_un un_addr;
   /* socket address is a pathname */
   char *socket_address = argv[1];
   time_t t;
   LinkedList routing_table;
   char buffer[BUFFER_SIZE];
   sync_msg_t msg;

   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;
   sa.sa_handler = sig_handler;

   if (sigaction(SIGINT, &sa, NULL) == -1) {
      perror("Problem with catching \"SIGINT\"");
      exit(EXIT_FAILURE);
   }

    
   server_socket = socket(AF_UNIX, SOCK_STREAM, 0);

   if (server_socket == -1) {
      perror("Server Socket Failure");
      exit(EXIT_FAILURE);
   }

   if (socket_address != NULL) {
      if (strlen(socket_address) > (sizeof(un_addr.sun_path) - 1)) {
         fprintf(stderr, "Server socket path too long\n");
         close(server_socket);
         exit(EXIT_FAILURE);
      }

      /* Remove any existing file with the same pathname for socket. */
      unlink(socket_address);

      /***********************************************************/
      /* Set socket to be nonblocking. All of the sockets for    */
      /* the incoming connections will also be nonblocking since */
      /* they will inherit that state from the listening socket. */
      /***********************************************************/

      flags = fcntl(server_socket, F_GETFL, 0);
       
      if (flags != -1) {
         retval = fcntl(server_socket, F_SETFL, flags | O_NONBLOCK);
         if (retval == -1) {
            perror("Fcntl Error");
            close(server_socket);
            exit(EXIT_FAILURE);
         }
      }
      else {
         perror("Fcntl Error");
         close(server_socket);
         exit(EXIT_FAILURE);
      }

      memset(&un_addr, 0, sizeof(struct sockaddr_un));
      un_addr.sun_family = AF_UNIX;
      strncpy(un_addr.sun_path, socket_address, (sizeof(un_addr.sun_path) - 1));

      if (bind(server_socket, (struct sockaddr *)&un_addr, sizeof(struct sockaddr_un)) == -1) {
         perror("Binding Failure");
         close(server_socket);
         exit(EXIT_FAILURE);
      }

      if (listen(server_socket, PEND_CON) == -1) {
         perror("Listening Failure");
         close(server_socket);
         exit(EXIT_FAILURE);
      }

      intit_client_handler_set();
      initTable(&routing_table);
      
      maxfd = server_socket;
      
      FD_ZERO(&workingset);
      FD_SET(STDIN_FILENO, &workingset);
      FD_SET(server_socket, &workingset);
      
      while (keeprunning) {
         printf("Make your choice (Insert, Update, Delete, Show):\n");
         FD_ZERO(&workingset);
         FD_SET(STDIN_FILENO, &workingset);
         FD_SET(server_socket, &workingset);
         ready = select(maxfd+1, &workingset, NULL, NULL, NULL);

         if (ready == -1) {
            if (errno !=EINTR) {
               perror("Select Failure");
               close(server_socket);
               unlink(socket_address);
               exit(EXIT_FAILURE);
            }
            else {
               continue; 
            }
         }
         
         if (FD_ISSET(server_socket, &workingset)) {
            do {
                /**********************************************/
                /* Accept each incoming connection.  If       */
                /* accept fails with EWOULDBLOCK, then we     */
                /* have accepted all of them.  Any other      */
                /* failure on accept will cause us to end the */
                /* server.                                    */
                /**********************************************/
                client_socket = accept(server_socket, NULL, NULL);
                  
                if (client_socket < 0) {
                   if (errno != EWOULDBLOCK) {
                      perror("Accept failed");
                      for (fdt = 0; fdt <= maxfd; fdt++) {
                         if (FD_ISSET(fdt, &workingset))
                              close(fdt);
                      }
                      unlink(socket_address);
                      exit(EXIT_FAILURE);
                    }
                    break;
                 }

                 add_to_client_handler_set(client_socket);
                 
                 printf("soco1:%d\n", client_socket);
                     
                 } while (client_socket != -1);
                 
                 printf("soco2\n");
                  
                 /*send the routing table to all clients*/
                 alignClients(&routing_table, NULL);
                  
            }
            
            if (FD_ISSET(STDIN_FILENO, &workingset)){
               /*input comes from stdin*/
               retval = read(STDIN_FILENO, buffer, BUFFER_SIZE);
               
               if (retval > 0) {
                  /* Remove trailing newline character from the input buffer if needed. */
                  buffer[retval-1] = '\0';
                  TableAction(buffer, &routing_table, &msg);
                   printf("soco3\n");
                  
                  if (msg.op_code != NOOPT) { 
                    alignClients(NULL, &msg);
                    printf("soco4\n");
                  }
                  
               }
            }
      }
      /* SIGINT has been received, so perform cleanup actions */
      for (fdt = 0; fdt <= maxfd; fdt++) {
         if (FD_ISSET(fdt, &workingset))
            close(fdt);
      }
      unlink(socket_address);
   }
   else { 
      printf("Wrong usage!\n");
      printf("Right Usage: ./unix_domain_server \"socket_path\" \n");
   }

   exit(EXIT_SUCCESS);
}
