#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "list.h"

void initialize_List(LinkedList **list) 
{

   (*list)->head = NULL;
   (*list)->tail = NULL;
   (*list)->nofids = 0;
}

void add_Head(LinkedList **list, void *dat) 
{

  if (*list != NULL) {
    Node *node = (Node *) malloc(sizeof(Node));
    node->tabledata = (data *) malloc(sizeof(data));
    
    if (node != NULL && node->tabledata == NULL) {
      perror("Malloc error inside function \"add_Head\" (tabledata element)");
      free(node);
      node = NULL;
      free(*list);
      *list = NULL;
      exit(EXIT_FAILURE);
    }
  
    if (node != NULL) {
      memcpy(node->tabledata, dat, sizeof(data));
 
      if ((*list)->head == NULL) {
        (*list)->tail = node;
        node->next = NULL;
      }
      else
        node->next = (*list)->head;
      
      node->id = ++((*list)->nofids);
      (*list)->head = node;
    }
    else {
      perror("Malloc error inside function \"add_Head\" (node element)");
      free(*list);
      *list = NULL;
      exit(EXIT_FAILURE);
    }
  }
}


Node *getNode(LinkedList **list, int nodenum) 
{
  Node *node = (*list)->head;
  
  if (nodenum > listNodeNum(list))
    return NULL;
  
  while ((node != NULL) && (nodenum != node->id))
      node = node->next;
  
  return node;
}



int listNodeNum(LinkedList **list) 
{
    
  if (*list != NULL)
    return (*list)->nofids;
  
  return 0;
}



