#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "utilities.h"
#include "routeTable.h"



void TableAction(char *entry, LinkedList *route_table, sync_msg_t *msg) 
{
    char *action, *part, *subpart, *mask;
    data entrydata;
    bool error = false;
    int result;
    
    if (entry == NULL) {
       if (route_table == NULL) {
         fprintf(stderr, "Null entry has been given!");
         return;
       }
       else {
         switch (msg->op_code) {
               case CREATE:
                 add_Head(&route_table, &msg->msg_body);
                 break;
               case UPDATE:
                 if (updateEntry(&route_table, msg->msg_body)) {
                   fprintf(stderr, "Problem with table entry update!");
                   return;
                 }
                 break;
               case DELETE:
                 printf("client:%s %d %s %s\n", msg->msg_body.destination, msg->msg_body.mask, msg->msg_body.gateway_ip, msg->msg_body.oif);
                 if (removeEntry(&route_table, msg->msg_body)) {
                   fprintf(stderr, "Problem with table entry removal!");
                   return;
                 }
                 updateAllIds(&route_table);
                 break;
               default:
                 break;
             }
          return;
       }
    }
    
    action = strtok(entry, " ");
    
    if (!strcmp(strlwr(action), "show")) {
      printAllentries(route_table);
      msg->op_code = NOOPT;
      return;
    }
    
    if (action == NULL || !strcmp(action, " "))
       return;
    
    if ((subpart = strtok(NULL, " ")) == NULL || !strcmp(subpart, " "))
       return;
    
    part = strdup(subpart);
    strcpy(entrydata.destination, strsep(&part, "/"));
    
    if ((mask = strsep(&part, "/")) == NULL)
       return;
    
    entrydata.mask = atoi(mask);
    
    if (strcmp(strlwr(action), "insert") == 0 
        || strcmp(strlwr(action), "update") == 0) {
      if ((part = strtok(NULL, " ")) != NULL)
        strcpy(entrydata.gateway_ip, part);
      else
        error = true;
      
      
      if ((part = strtok(NULL, " ")) != NULL)
        strcpy(entrydata.oif, part);
      else
        error = true;
      
      if (!error) {
        if (strcmp(strlwr(action), "insert") == 0) {
          if (checkForDoubleEntries(route_table, entrydata)) {
            add_Head(&route_table, &entrydata);
            msg->op_code = CREATE;
            memcpy(&msg->msg_body, &entrydata, sizeof(data));
          }
          else {
            printf("Entry already in the table\n");
            msg->op_code = NOOPT;
          }
        }
        else {
          if (updateEntry(&route_table, entrydata))
            fprintf(stderr, "Problem with table entry update!");
          else {
            msg->op_code = UPDATE;
            memcpy(&msg->msg_body, &entrydata, sizeof(data));
          }
        }
      }
      else
        fprintf(stderr, "Problem with user's input!");
    }
    else if (strcmp(strlwr(action), "delete") == 0) {
      if (removeEntry(&route_table, entrydata))
        fprintf(stderr, "Problem with table entry removal!");
      else {
        updateAllIds(&route_table);
        msg->op_code = DELETE;
        memcpy(&msg->msg_body, &entrydata, sizeof(data));
      }
    }
    else {
      fprintf(stderr, "Non supported option");
      msg->op_code = NOOPT;
      return;
    }
}

void initTable(LinkedList *route_table)
{
  initialize_List(&route_table);  
}


void printAllentries(LinkedList *route_table) {
  Node *currentNode;
  int i;
  
  printf("listNodeNum:%d\n", listNodeNum(&route_table));
  
  printf("Destination subnet | Gateway IP | OIF\n");
  
  for (i=1; i<=listNodeNum(&route_table); i++) {
    currentNode = getNode(&route_table, i);
     printf("node:%p\n", currentNode);
    if (currentNode != NULL) {
    printf("%s/%d\t\t\t %s\t %s\n",currentNode->tabledata->destination, 
           currentNode->tabledata->mask, currentNode->tabledata->gateway_ip,
           currentNode->tabledata->oif);
    }
  }
    
}


void cleanUpTable(LinkedList *route_table) 
{
  Node *tmp, *current;
  
  if (route_table->head == NULL) {
    return;
  }
  else if (route_table->head == route_table->tail) {
    tmp = route_table->head;
    route_table->head =  route_table->tail = NULL;
    free(tmp->tabledata);
    tmp->tabledata = NULL;
    free(tmp);
    tmp = NULL;
  }
  else {
    tmp = route_table->head;
    
    while(tmp != NULL) {
      current = tmp;
      free(current->tabledata);
      current->tabledata = NULL;
      free(current);
      current = NULL;
      tmp = tmp->next;
    }
  }
}


int updateEntry(LinkedList **route_table, data entrydata)
{
  int i;
  Node *currentnode = (*route_table)->head;
    
  while (currentnode != NULL) {
    if ((!strcmp(currentnode->tabledata->destination, entrydata.destination)) 
        && (currentnode->tabledata->mask == entrydata.mask)) {
        strcpy(currentnode->tabledata->gateway_ip, entrydata.gateway_ip);
        strcpy(currentnode->tabledata->oif, entrydata.oif);
        return 0;
      }
     currentnode = currentnode->next;
  }    
  
  return 1;
  
}


data *getEntry(LinkedList **route_table, int entryidx)
{
  Node *currentNode;
  
  currentNode = getNode(route_table, entryidx);
   
  return (currentNode != NULL) ? currentNode->tabledata : NULL;
}

int checkForDoubleEntries(LinkedList *route_table, data entrydata)
{
  Node *node = route_table->head;
    
  while (node != NULL) {
    if ((!strcmp(node->tabledata->destination, entrydata.destination)) 
        && (node->tabledata->mask == entrydata.mask)) {
        return 0;
      }
      node = node->next;
  }    
  return 1;  
 }
 
 
 static void updateAllIds(LinkedList **route_table) {
   Node *node = (*route_table)->head;
   int count = 1;
   
   printf("Inside updateAllIds\n");
   
   while(node != NULL) {
     node->id = count++;
     printf("node:%p\n", node);
     node = node->next;
   }
 }
 

 
 int removeEntry(LinkedList **route_table, data entrydata) 
 {
    Node *tmp, *node = (*route_table)->head;
    bool found = false;
    
    while (node != NULL) {
      if (!strcmp(node->tabledata->destination, entrydata.destination) 
          && (node->tabledata->mask == entrydata.mask)) {
        found = true;
        break;
      }
      node = node->next;
    }

    if (found) {
      if (node == (*route_table)->head) {
        if ((*route_table)->head->next == NULL)
          (*route_table)->head = (*route_table)->tail = NULL;
        else
          (*route_table)->head = (*route_table)->head->next;
      }
      else {
        /*You must find the previous node*/
        tmp = (*route_table)->head;
        while (tmp != NULL && tmp->next != node) 
          tmp = tmp->next;
        
        if (tmp != NULL)
          tmp->next = node->next;
      }
    
      free(node->tabledata);
      node->tabledata = NULL;
      free(node);
      node = NULL;
      -- (*route_table)->nofids;
    
      return 0;
    }
    
    return 1;
   }
