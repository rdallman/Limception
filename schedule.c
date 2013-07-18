#include <stdio.h>




typedef struct Node {
  Node next;
  int priority;
  char[10] name;
  int start_time;
  int cpu_time;
  int io_count;
  int completion_time;
  int time_slice;
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

Node pop(Queue* q) {
}

void * runExponential(int count, Node n) {
  int done = count + n.time_slice;
  while count < done {
    if (io_done) {
      if ((done - count) < n.time_slice / 2) {
        if (n.priority < 8) {
          n.priority += 1;
        }
        n.time_slice = n.time_slice / 2;
      }
    }
    count++;
  }
  if (n.priority > 1) {
    n.priority -= 1;
  }
  n.time_slice = n.time_slice * 2;
}

void * runSTCF(Node n)  {
}

int main() {
  return 0
}
