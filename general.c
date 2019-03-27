void initializeList(LinkedList **list) {

   *list = (LinkedList *)malloc(sizeof(LinkedList));

   if (*list == NULL) {
      perror("Memory allocation error!");
      exit(EXIT_FAILURE);
   }
   (*list)->head = NULL;
   (*list)->tail = NULL;
}
