#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int new_process;

typedef struct Node {
  struct Node *next;
  int priority;
  char name[10];
  int start_time;
  int cpu_time;
  int io_count;
  int io_blocks_left;
  int cpu_completed;
  int completion_time;
  int time_slice;
} Node;

typedef struct Queue {
  Node *head;
  Node *tail;
} Queue;

void * push_exponential(Queue* q, Node *n) {
  Node *insert = q->head;
  while (insert->priority >= n->priority) {
    insert = insert->next;
  }
  Node* next = insert;
  insert = n;
  n->next = next;
}

void * push_stcf(Queue* q, Node *n) {
  Node *insert = q->head;
  while (insert->cpu_time >= n->cpu_time) {
    insert = insert->next;
  }
  Node *next = insert;
  insert = n;
  n->next = next;
}

Node * pop(Queue* q) {
  Node* head = q->head;
  q->head = head->next;
  return head;
}

int run(int clock, Node *n) {
  //context
  clock++;

  //make sure this works for STCF
  int done = clock + n->time_slice;
  int block_time = n->cpu_time / n->io_blocks_left;
  int next_io = block_time;
  while (clock < done && n->cpu_completed < n->cpu_time || n->io_blocks_left > 0) {
    //interrupt
    if (new_process) {
      new_process = 0;
      if ((done - clock) < (n->time_slice / 2)) {
        if (n->priority < 8) {
          n->priority += 1;
        }
        n->time_slice = n->time_slice / 2;
      }
      break;
    }
    if (n->cpu_completed < n->cpu_time) {
      clock++;
    }
    //IO
    if (next_io == 0 || n->cpu_completed == n->cpu_time) {
      int done_io = clock + 10;
      if (n->io_blocks_left > 0) {
        while (clock < done_io) {
          clock++;
        }
        next_io = block_time;
        n->io_blocks_left--;
      }
      if (n->io_blocks_left > 0 && n->cpu_completed == n->cpu_time) {
        return clock;
      }
    }
    next_io--;
  }
  //exponential
  if (clock == done) {
    if (n->priority > 1) {
      n->priority -= 1;
    }
    n->time_slice = n->time_slice * 2;
  }
  //context
  clock++;
  return clock;
}

Node read_trace_line() {
  Node *n = (Node*) malloc(sizeof(Node));
  //n.name = get(name)
  //n.start_time = get(start)
  //n.cpu_time = get(cpu) * 1000
  //n.io_count = get(io)
  n->io_blocks_left = trunc((n->io_count + 8191) / 8192);
  n->completion_time = 0;
  int exp = 1;
  if (exp) {
    n->time_slice = 10;
  } else {
    n->time_slice = n->cpu_time;
  }
}

int main() {
  //pop
  //run
  //cpu_time != compl_time?  push(ready)
  return 0;
}
