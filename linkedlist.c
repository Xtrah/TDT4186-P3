// Source: https://github.com/skorks/c-linked-list

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"

struct node {
  int pid;
  char command[1024];
  struct node * next;
};

struct list {
  Node * head;
};

char *getcommand(Node *node) {
  return node->command;
}

Node * createnode(int pid, char *command){
  Node * newNode = malloc(sizeof(Node));
  if (!newNode) {
    return NULL;
  }
  newNode->pid = pid;
  int size = sizeof(char) * sizeof(command);
  strncpy(newNode->command, command, size);
  newNode->command[size] = '\0'; // ensure termination

  newNode->next = NULL;
  return newNode;
}

List * makelist(){
  List * list = malloc(sizeof(List));
  if (!list) {
    return NULL;
  }
  list->head = NULL;
  return list;
}

void display(List * list) {
  Node * current = list->head;
  if(list->head == NULL) 
    return;
  
  for(; current != NULL; current = current->next) {
    printf("PID: %d, CMD: %s\n", current->pid, current->command);
  }
}

Node *findnode(List * list, int pid) {
  Node * current = list->head;
  if(list->head == NULL) 
    return NULL;
  
  for(; current != NULL; current = current->next) {
    if(current->pid == pid) {
      return current;
    }
  }
  return NULL;
}

void add(int pid, char *command, List * list){
  Node * current = NULL;
  if(list->head == NULL){
    list->head = createnode(pid, command);
  }
  else {
    current = list->head; 
    while (current->next!=NULL){
      current = current->next;
    }
    current->next = createnode(pid, command);
  }
}

void delete(int pid, List * list){
  Node * current = list->head;            
  Node * previous = current;           
  while(current != NULL){           
    if(current->pid == pid){      
      previous->next = current->next;
      if(current == list->head)
        list->head = current->next;
      
      free(current);
      return;
    }                               
    previous = current;             
    current = current->next;        
  }                                 
}

void destroy(List * list){
  Node * current = list->head;
  Node * next = current;
  while(current != NULL){
    next = current->next;
    free(current->command);
    free(current);
    current = next;
  }
  free(list);
}