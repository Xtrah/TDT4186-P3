// Source: https://github.com/skorks/c-linked-list

#ifndef LINKEDLIST_HEADER
#define LINKEDLIST_HEADER

typedef struct node Node;

typedef struct list List;

List * makelist();
char *getcommand(Node *node);
void add(int pid, char *command, List * list);
void delete(int pid, List * list);
void display(List * list);
Node *findnode(List * list, int pid);
// void reverse(List * list);
// void reverse_using_two_pointers(List * list);
void destroy(List * list);

#endif