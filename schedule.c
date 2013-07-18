#include <stdio.h>




typedef struct Node {
  Node next;
  int priority;
  char[10] name;
  int start_time;
  int cpu_time;
  int io_count;
  int completion_time;
} Node;

typedef struct Queue {
  Node head;
  Node tail;
} Queue;

void * pushExponential(Queue* q, Node n) {
  Node insert = q->head;
  while insert.priority >= n.priority {
    insert = insert.next;
  }
}

Node popExponential(Queue* q) {
}


int main() {
  return 0
}
