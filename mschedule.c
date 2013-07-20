//http://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer
//http://linux.die.net/man/3/strtok_r
#define _XOPEN_SOURCE 600
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Node {
  struct Node *next;
  int priority;
  char *name;
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
  int size;

  void (*push_wait) (struct Queue* q, Node *n);
  Node *(*peek) (struct Queue* q);
  void (*push_exponential) (struct Queue* q, Node *n);
  void (*push_stcf) (struct Queue* q, Node *n);
  Node *(*pop) (struct Queue* q);
} Queue;

void * push_exponential(Queue* q, Node *n) {
  /*
  if (q->head) {
    Node *insert = q->head;
    while (insert->next && insert->priority >= n->priority) {
      insert = insert->next;
    }
    Node *next = insert;
    insert = n;
    n->next = insert;
  } else {
    n->next = NULL;
    q->head = n;
  }
  */
  printf("idiot");
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

Node * peek(Queue* q){
  return q->head;
}

void * push_wait(Queue* q, Node *n){
  Node *insert = q->head; //malloc??
  printf("Pushing: %s\tstart_time: %d\tcpu_time: %d\tio_count: %d\n", n->name, n->start_time, n->cpu_time, n->io_count);
  if(q->peek(q)) {
    int m = 0;
    while (insert->start_time >= n->start_time) {
      m++;
      insert = insert->next;
    }
  } else {
    q->head = n;
  }
  q->size++;
}

Queue wq, rq, dq; //wait queue, ready queue
int new_process;

void * exponentialHold() {
  struct timeval tv;
  int time;
  while(wq.peek(&wq)){
    gettimeofday(&tv, NULL);
    time = ((tv.tv_sec % 86400) * 1000 + tv.tv_usec / 1000);
    printf("\n%d", time);
    if (wq.peek(&wq)) {
      rq.push_exponential(&rq, wq.pop(&wq));
    }
    if(time == wq.head->start_time) {
      printf("%d", time);
      rq.push_exponential(&rq, wq.pop(&wq));
    }
  }
}

void * exponentialReady() {
  int clock = 0;
  int wait = 0;
  while (wq.peek(&wq) || rq.peek(&rq)) {
    clock++;
    wait++;
    if (rq.peek(&rq)) {
      Node *worker = rq.pop(&rq);
      clock = run(clock, &worker);
      if (worker->cpu_time == worker->completion_time) {
        dq.push_wait(&dq, &worker);
      } else {
        rq.push_wait(&rq, &worker);
      }
    }
  }
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

//Node read_trace_line() {
 // Node *n = (Node*) malloc(sizeof(Node));
  //n.name = get(name)
  //n.start_time = get(start)
  //n.cpu_time = get(cpu) * 1000
  //n.io_count = get(io)
 // n->io_blocks_left = trunc((n->io_count + 8191) / 8192);
 // n->completion_time = 0;
 // int exp = 1;
 // if (exp) {
 //   n->time_slice = 10;
 // } else {
 //   n->time_slice = n->cpu_time;
 // }
//}

int main(int argc, char *argv[]) {
  wq.size = 0;
  wq.head = NULL;
  wq.tail = NULL;
  wq.push_wait = &push_wait;
  wq.peek = &peek;

  rq.size = 0;
  rq.head = NULL;
  rq.tail = NULL;
  rq.push_exponential = &push_exponential;
  rq.peek = &peek;

  dq.size = 0;
  dq.head = NULL;
  dq.tail = NULL;
  dq.push_wait = &push_wait;
  dq.peek = &peek;


  FILE *file = fopen(argv[1], "r");
  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  fseek(file, 0, SEEK_SET);
  char *string  =malloc(fsize+1);
  fread(string, fsize, 1, file);
  fclose(file);

  string[fsize] = 0;

  //add stuff to node -> waiting queue
  char *token, *saveptr,*saveptr2, *temp, *ptr_end;
  int j, k, a;
  for(j = 0; ; j++, string = NULL){
    token = strtok_r(string, "\n", &saveptr);
    if (token == NULL) {
      break;
    }
    Node *n = (Node*)malloc(sizeof(Node));
    n->name = malloc(11);

    for (k = 0; ; k++, token = NULL){
      temp = strtok_r(token, "\' \', \t", &saveptr2);
      if (temp == NULL) {
        break;
      }
      if(k == 0) {
        strcpy(n->name, temp);
      }
      else if(k == 1) {
        a = atoi(temp);
        n->start_time = a;
      }
      else if(k == 2) {
        a = atof(temp);
        n->cpu_time = a * 100;
      }
      else {
        a = atof(temp);
        n->io_count = a;
        n->io_blocks_left = trunc((n->io_count + 8191) / 8192);
        n->completion_time = 0;
        int exp = 1;
        if (exp) {
          n->time_slice = 10;
        } else {
          n->time_slice = n->cpu_time;
        }
        wq.push_wait(&wq, n);
      }
    }
  }

  // wait for things to go down
  pthread_t waiting;
  pthread_t ready;
  if (pthread_create(&waiting, NULL, &exponentialHold, NULL)){
    printf("Could not create thread \n");
  }
  if (pthread_create(&ready, NULL, &exponentialReady, NULL)) {
    printf("Could not create thread \n");
  }
  if(pthread_join(waiting, NULL)){
    printf("Could not join thread\n");
  }
  if(pthread_join(ready, NULL)) {
    printf("Could not join thread\n");
  }
  //printf("%s", dq.head->name);
  //pop
  //run
  //cpu_time != compl_time?  push(ready)
  return 0;

}
