/*************************************************************************
 * Intracom Telecom
 * Project: Control Plane Software (CPS) framework
 * Sub-project: CPS library
 * Filename: unix_domain_server.c
 * Description: UNIX Domain Server
 ************************************************************************/

#include <stdio.h>
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
#include "msg_utilities.h"

#define TIME_INTERVAL 10
#define PEND_CON 12 /* Indicates the number of pending connections for listen function. */

typedef struct _thread_data {
   struct timeval timeout;
} thread_data;

const char *responsemessage = "Hello from server!";
static volatile sig_atomic_t keeprunning = 1; /* controlling server process state (running / terminated) */
pthread_t thread_id;
static int pfd[2], shared_variable = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void cleanUpHandler(void *arg) {
   printf("Do some cleanup stuff and terminate!\n");
   if (close(pfd[0]) < 0) {
       perror("Problem during closure of read end");
   } 

   if (close(pfd[1]) < 0) {
       perror("Problem during closure of write end");
   }
}

void sig_handler(int sig) {
    if (sig == SIGINT) {
       keeprunning = 0; /* stop the server process currently running */
       if (pthread_cancel(thread_id) != 0)
          perror("Problem during cancelling thread!");
       fprintf(stderr, "Stopping server process!\n");
    }
}

void * thread_func(void *arg) {
   int ready, maxfd;
   thread_data *th_data; 
   unsigned int tv_secs, tv_usecs;
   fd_set readfdset;
   char option;

   th_data = (thread_data *)arg;
   maxfd = pfd[0];
   tv_secs = th_data->timeout.tv_sec;
   tv_usecs = th_data->timeout.tv_usec;
   FD_ZERO(&readfdset);
   FD_SET(pfd[0], &readfdset);
   pthread_cleanup_push(cleanUpHandler, NULL);

   while (1) {
      FD_ZERO(&readfdset);
      FD_SET(pfd[0], &readfdset);
      ready = select(maxfd+1, &readfdset, NULL, NULL, &th_data->timeout); /* select syscall will be one cancelling point */
      if (ready == 0) {
         printf("Hello from thread:%ld\n", pthread_self());
         /* refresh timeout value of select */
         th_data->timeout.tv_sec = tv_secs;
         th_data->timeout.tv_usec = tv_usecs;
         continue; 
      }
      else if (ready == -1) {
         perror("Select Failure");
         cleanUpHandler(NULL);
         return NULL;
      }
      /* an fd is ready */
      else {
         if (FD_ISSET(pfd[0], &readfdset)) {
            while (1) {
               if (read(pfd[0], &option, 1) > 0) { /* this is another cancellation point */
                  if (option == 'r') {
                     if (pthread_mutex_lock(&mtx) != 0) {
                        perror("Mutex lock error!");
                        cleanUpHandler(NULL);
                        return NULL;
                     }

                     printf("Thread reads \"shared_variable\" value:%d\n", shared_variable);

                     if (pthread_mutex_unlock(&mtx) != 0) {
                        perror("Mutex unlock error!");
                        cleanUpHandler(NULL);
                        return NULL;
                     }
                  }
                  else if (option == 'w') {
                     if (pthread_mutex_lock(&mtx) != 0) {
                        perror("Mutex lock error!");
                        cleanUpHandler(NULL);
                        return NULL;
                     }

                     ++ shared_variable;
                     printf("Thread updates \"shared_variable\" to value:%d\n", shared_variable);
                     
                     if (pthread_mutex_unlock(&mtx) != 0) {
                        perror("Mutex unlock error!");
                        cleanUpHandler(NULL);
                        return NULL;
                     }
                  }
                  else { 
                     printf("Unrecognized option\n");
                  }
               }
               else {
                  /* no more bytes to read */
                  if (errno == EAGAIN)
                     break;
                  else {
                     perror("Read failure!");
                     cleanUpHandler(NULL);
                     return NULL;
                  }
               }
            }
         }
      }
   }
   pthread_cleanup_pop(1);
   return NULL;
}

int main(int argc, char *argv[]) {
   /* structure for providing timeout on select */
   struct timeval timeout;
   int i, maxfd, fd, client_socket, ready, server_socket, flags, retval, fdt, bytes, random_pos;
   bool close_con = false;
   char request[256], ch, option[2];
   /* define a structure for de-serializing data */
   ser_buf_t *buf;
   fd_set masterfdset, workingset;
   /* UNIX domain socket structure */
   struct sockaddr_un un_addr;
   /* socket address is a pathname */
   char *socket_address = argv[1], *options[] = {"r", "w"};
   struct sigaction sa;
   /* it is used when calling thread start function */
   thread_data thdata;
   void *result;
   time_t t;

   /* Intializes random number generator */
   srand((unsigned) time(&t));

   /* setting up the timeout to 3 mins */
   timeout.tv_sec = 3 * TIME_INTERVAL;
   timeout.tv_usec = 0;

   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;
   sa.sa_handler = sig_handler;

   if (sigaction(SIGINT, &sa, NULL) == -1) {
       perror("Problem with catching \"SIGINT\"");
       exit(EXIT_FAILURE);
   }

   if (pipe(pfd) == -1) {
      perror("Problem with pipe creation");
      exit(EXIT_FAILURE); 
   }

   /* Make read end of pipe non-blocking */
   flags = fcntl(pfd[0], F_GETFL);
   if (flags != -1) {
      retval = fcntl(pfd[0], F_SETFL, flags | O_NONBLOCK);
      if (retval == -1) {
         perror("Fcntl Error");
         exit(EXIT_FAILURE);
      }
   }
   else {
       perror("Fcntl Error");
       exit(EXIT_FAILURE);
   }

   /* Make write end of pipe non-blocking */
   flags = fcntl(pfd[1], F_GETFL);
   if (flags != -1) {
      retval = fcntl(pfd[1], F_SETFL, flags | O_NONBLOCK);
      if (retval == -1) {
         perror("Fcntl Error");
         exit(EXIT_FAILURE);
      }
   }
   else {
       perror("Fcntl Error");
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

      FD_ZERO(&masterfdset);
      FD_SET(server_socket, &masterfdset);
      maxfd = server_socket;
      /* create the thread */
      thdata.timeout.tv_sec = TIME_INTERVAL;
      thdata.timeout.tv_usec = 0;

      if (pthread_create(&thread_id, NULL, thread_func, &thdata) != 0) {
         perror("Error during thread creation!");
         close(server_socket);
         exit(EXIT_FAILURE); 
      }

      while (keeprunning) {
         memcpy(&workingset, &masterfdset, sizeof(masterfdset));
         printf("Waiting for a client connection!\n");
         ready = select(maxfd+1, &workingset, NULL, NULL, &timeout);

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
         else if (ready == 0) {
            /*fprintf(stderr,"Timeout happened before any descriptor becomes ready!\n");
            close(server_socket);
            unlink(socket_address);
            exit(EXIT_SUCCESS);*/
            timeout.tv_sec = 3 * TIME_INTERVAL;
            timeout.tv_usec = 0;

            if (pthread_mutex_lock(&mtx) != 0) {
               perror("Mutex lock error!");
               close(server_socket);
               unlink(socket_address);
               exit(EXIT_FAILURE);
            }

            ++ shared_variable; /* update variable's value */
            printf("Main thread updates \"shared_variable\" to:%d value\n",shared_variable);

            if (pthread_mutex_unlock(&mtx) != 0) {
               perror("Mutex unlock error!");
               close(server_socket);
               unlink(socket_address);
               exit(EXIT_FAILURE);
            }
            continue;

         }
         
         for (i = 0; i <= maxfd; i++) {
		    if (FD_ISSET(i, &workingset)) {
               if (i == server_socket) {
                  printf("Master socket is readable:%d\n", i);
                
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
                              if (FD_ISSET(fdt, &masterfdset))
                                 close(fdt);
                           }
                           unlink(socket_address);
                           exit(EXIT_FAILURE);
                        }
                        break;
                     }

                     printf("New incoming connection - %d\n", client_socket);
                     FD_SET(client_socket, &masterfdset);
 
                     if (client_socket > maxfd)
                        maxfd = client_socket;
                  } while (client_socket != -1);                 
                }
                else {
                   close_con = false;
                          
                   while (1) {
                      bytes = recv(i,request,sizeof(request),0);

                      if (bytes > 0) {
                         strcpy(option, options[rand() % 2]);
                         printf("\n\nOption from main thread:%s\n\n", option);
                         if (write(pfd[1], option, 1) > 0 && errno != EAGAIN) {
                            perror("Write problem");
                            close_con = true;
                            break; 
                         }
                         printf("Request from client socket %d is received...\n",i);

                         /* decoding the request received */
                         printf("**********************Decoding*************************\n");
                         decode(request);
                         printf("*******************************************************\n");

                         if (send(i, responsemessage, (strlen(responsemessage) + 1), 0) > 0)
                            printf("Replying back to client\n");
                         else {
                            perror("Send Problem");
                            close_con = true;
                            break;
                         }
                      }
                       /**************************************************************/
                       /*  If no messages are available to be received and the peer  */
                       /*  has performed an orderly shutdown, recv() shall return 0. */
                       /**************************************************************/
				          else if (bytes == 0) {
                         fprintf(stderr, "Connection from client has been closed\n");
			                close_con = true;
                         break;
                      }
                      else {
                         if (errno != EWOULDBLOCK) {
                            perror("Server Receive Problem");
                            close_con = true;
                         }
                         break;
                      }
                   }

                   /******************************************************************/
                   /*   Remove the descriptor from working set and close it at the   */ 
                   /*   same time, moreover we should update maxfd value if needed   */
                   /******************************************************************/
                   if (close_con) {
                      close(i);
                      FD_CLR(i, &masterfdset);

			             if (i == maxfd) {
                         while (!FD_ISSET(maxfd, &workingset))
                         maxfd -= 1;
                      }
                  }
                }
            }
         }
      }
      /* SIGINT has been received, so perform cleanup actions */
      for (fdt = 0; fdt <= maxfd; fdt++) {
         if (FD_ISSET(fdt, &masterfdset))
            close(fdt);
      }
      unlink(socket_address);
      if (pthread_join(thread_id, &result) != 0) {
         perror("Join failure!");
         exit(EXIT_FAILURE);
      }

      if (result == PTHREAD_CANCELED)
         printf("Thread has been cancelled!\n");
      /* Never reach here */
      else
         printf("Thread has not been cancelled (other result)!");     
   }
   else { 
      printf("Wrong usage!\n");
      printf("Right Usage: ./unix_domain_server \"socket_path\" \n");
   }

   exit(EXIT_SUCCESS);
}
