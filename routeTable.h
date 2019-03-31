#include "list.h"

void TableAction(char *entry, LinkedList *route_table, sync_msg_t *msg);

void initTable(LinkedList *route_table);

int updateEntry(LinkedList **route_table, data entrydata);

int findNode(LinkedList **route_table, data entrydata, Node **node);

int removeEntry(LinkedList **route_table, data entrydata);

data *getEntry(LinkedList **route_table, int entryidx);

void printAllentries(LinkedList *route_table);

static void updateAllIds(LinkedList **route_table);

int checkForDoubleEntries(LinkedList *route_table, data entrydata);

void cleanUpTable(LinkedList *route_table);
