/*************************************************************************
 * Intracom Telecom
 * Project: Control Plane Software (CPS) framework
 * Sub-project: CPS library
 * Filename: ser_des_api.h
 * Description: SERIALIZE AND DESERIALIZE API
 ************************************************************************/

#include "ser_des_api.h"
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

void init_ser_buf(ser_buf_t **buf) {

   *buf = (ser_buf_t *)malloc(sizeof(ser_buf_t));

   if (*buf == NULL) {
      perror("Mem alloc error for \"buf\"");
      exit(EXIT_FAILURE);
   }

   (*buf)->b = (char *)malloc(MAX_BUF_SIZE);
 
   if ((*buf)->b == NULL) {
       perror("Mem alloc error for \"b\" field");
       free(*buf);
       exit(EXIT_FAILURE);
   }

   (*buf)->size = MAX_BUF_SIZE;
   (*buf)->next = 0; 
}

void ser_data(ser_buf_t **buf, void *data, int nbytes){

   int available;
   bool resized;

   available = (*buf)->size - (*buf)->next;
   resized = false;
   /********************************************************
    * Buffer size is doubled until the available space is  *
    * enough to host the "nbytes" data                     *
    ********************************************************/
   while (available < nbytes) {
      (*buf)->size *= 2;
      available = (*buf)->size - (*buf)->next;
      resized =  true;
   }

   if (!resized) {
      memcpy((*buf)->b + (*buf)->next, data, nbytes);
      (*buf)->next += nbytes; /* advance the pointer to indicate the next free position */
   }
   else {
      (*buf)->b = (char *)realloc((*buf)->b, (*buf)->size);

      if ((*buf)->b == NULL) {
         perror("Mem alloc error inside \"ser_data\" function");
         exit(EXIT_FAILURE);
      }

      memcpy((*buf)->b + (*buf)->next, data, nbytes);
      (*buf)->next += nbytes;
   }
}

void deser_data(void *dest, ser_buf_t **buf, int nbytes){
   memcpy(dest,(*buf)->b + (*buf)->next, nbytes);
   (*buf)->next += nbytes;
}

void dealloc_ser_buf(ser_buf_t **buf){
   if (*buf != NULL) {
      free((*buf)->b);
      (*buf)->b = NULL;
      free(*buf);
      *buf = NULL;
   }
}
