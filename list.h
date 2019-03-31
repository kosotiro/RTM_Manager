#include "general.h"

typedef struct _node {
   data *tabledata;
   int id;
   struct _node *next;
} Node;


typedef struct _linkedlist {
   Node *head;
   Node *tail;
   int nofids;
} LinkedList;


/* initialize list */
void initialize_List(LinkedList **list);
void add_Head(LinkedList **list, data *dat);
Node *getNode(LinkedList **list, int nodenum);
int listNodeNum(LinkedList **list);
