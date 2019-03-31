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

void add_Head(LinkedList **list, data *dat) 
{
  Node *currentNode, *node;
  if (*list != NULL) {
    if (listNodeNum(list) > 0 ) {
        currentNode = (*list)->head;
        while (currentNode != NULL && currentNode->next != currentNode) {
          if ((!strcmp(currentNode->tabledata->destination, dat->destination)) 
             && (currentNode->tabledata->mask == dat->mask)) {
            return;
          }
          currentNode = currentNode->next;
        }
    }
    
    node = (Node *) malloc(sizeof(Node));
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
  
  return (nodenum == node->id) ? node : NULL;
}



int listNodeNum(LinkedList **list) 
{
    
  if (*list != NULL)
    return (*list)->nofids;
  
  return 0;
}



