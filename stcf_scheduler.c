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
  int completion_time;
  int cpu_time;
  int io_count;
  int io_blocks_left;
  int io_block_time;
  int io_block_next;
  int cpu_completed;
  int time_slice;
} Node;

typedef struct Queue {
  Node *head;
  Node *tail;
  int size;

  void *(*push_wait) (struct Queue* q, Node* n);
  Node *(*pop) (struct Queue* q);
  Node *(*peek) (struct Queue* q);
  void *(*push_stcf) (struct Queue* q, Node *n);
} Queue;

int mClock;
int mWait;

void * push_stcf(Queue* q, Node *n) {
  if(q->peek(q)) {
    Node *insert = q->head;
    while (insert->next && insert->next->cpu_time >= n->cpu_time) {
      insert = insert->next;
    }
    Node *next = insert->next;
    insert->next = n;
    n->next = next;
  } else {
    q->head = n;
    n->next = NULL;
  }
  q->size++;
}

Node * peek(Queue* q){
  return q->head;
}

Node * pop(Queue* q) {
  Node *head = q->head;
  q->head = head->next;
  q->size--;
  return head;
}

void * push_wait(Queue* q, Node *n){
  Node *insert = q->head; //malloc??
  printf("Pushing: %s\tstart_time: %d\tcpu_time: %d\tio_count: %d\n", n->name, n->start_time, n->cpu_time, n->io_count);
  if(q->peek(q)) {
    int m = 0;
    while (insert->next && insert->next->start_time <= n->start_time) {
      m++;
      insert = insert->next;
    }
    Node *next = insert->next;
    insert->next = n;
    n->next = next;
  } else {
    q->head = n;
    n->next = NULL;
  }
  q->size++;
}

Queue wq, rq, dq; //wait queue, ready queue, done queue
int new_process;

void * stcfHold() {
  struct timeval tv;
  int time;
  while(wq.peek(&wq)){
    gettimeofday(&tv, NULL);
    time = ((tv.tv_sec % 86400) * 1000 + tv.tv_usec / 1000);
    if(time == wq.peek(&wq)->start_time) {

      printf("\nThis\n %s", wq.peek(&wq)->name);

      rq.push_stcf(&rq, wq.pop(&wq));
      new_process = 1;
    }
  }
}

void * stcfReady() {
  mClock = 0;
  mWait = 0;

  if (rq.peek(&rq)){
  printf("EXPO READY %s", rq.peek(&rq)->name);}


  while (wq.peek(&wq) || rq.peek(&rq)) {
    if (rq.peek(&rq)) {
      printf("PEEK%s", rq.peek(&rq)->name);
    }
    mClock++;
    mWait++;
    if (rq.peek(&rq)) {
      Node *worker = rq.pop(&rq);

      printf("before RUN with %s", worker->name);

      mClock = run(mClock, worker);
      printf(" %d", worker->cpu_completed);
      printf(" / %d", worker->cpu_time);
      printf("\n");
      if (worker->cpu_time == worker->cpu_completed) {
        //dq.push_wait(&dq, &worker);
        struct timeval tv;
        int time;
        gettimeofday(&tv, NULL);
        time = ((tv.tv_sec % 86400) * 1000 + tv.tv_usec / 1000);
        worker->completion_time = time;

        printf("done");
      } else {
        rq.push_stcf(&rq, worker);
      }
    }
  }
}

int run(int clock, Node *n) {

  //make sure this works for STCF
  int done = clock + n->cpu_time;
  while (clock < done) {
    //interrupt
    if (new_process) {
      new_process = 0;
      printf("INTERRUPT");
      break;
    }
    if (n->cpu_completed < n->cpu_time) {
      //printf("total %d", n->cpu_completed);
      //printf("comp / %d", n->cpu_time);
      n->cpu_completed++;
      clock++;
      n->io_block_next--;
    }
    //IO
    if (n->io_block_next == 0 || n->cpu_completed == n->cpu_time) {
      if (n->io_blocks_left > 0) {
        int done_io = clock + 10;
        printf("\nIO blocks left %d", n->io_blocks_left);
        printf("\n");
        while (clock < done_io) {
          clock++;
        }
        n->io_block_next = n->io_block_time;
        n->io_blocks_left--;
      }
      if (n->io_blocks_left > 0 && n->cpu_completed == n->cpu_time) {
        return clock;
      }
    }
    if (n->cpu_completed == n->cpu_time) {
      break;
    }
  }
  return clock;
}

int main(int argc, char *argv[]) {
  wq.size = 0;
  wq.head = NULL;
  wq.tail = NULL;
  wq.push_wait = &push_wait;
  wq.peek = &peek;
  wq.pop = &pop;

  rq.size = 0;
  rq.head = NULL;
  rq.tail = NULL;
  rq.push_stcf = &push_stcf;
  rq.peek = &peek;
  rq.pop = &pop;

  dq.size = 0;
  dq.head = NULL;
  dq.tail = NULL;
  dq.push_wait = &push_wait;
  dq.peek = &peek;
  dq.pop = &pop;



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
    //get current time + x
    struct timeval tv;
    int time;
    gettimeofday(&tv, NULL);
    time = ((tv.tv_sec % 86400) * 1000 + tv.tv_usec / 1000);
    //printf("current%d", time);
    time += 1000 + j*100;

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
        /* actual
        a = atoi(temp);
        n->start_time = a;
        *//*testing*/ 
        n->start_time = time;
      }
      else if(k == 2) {
        a = atoi(temp);
        n->cpu_time = a * 1000;
      }
      else {
        a = atoi(temp);
        n->io_count = a;
        n->io_blocks_left = trunc((n->io_count + 8191) / 8192);
        printf("io blocks left = %d+8191 div 8192 = %d", n->io_count, n->io_blocks_left);
        n->priority = 1;
        n->cpu_completed= 0;
        n->io_block_time = n->cpu_time / n->io_blocks_left;
        n->io_block_next = n->io_block_time;
        printf("\nblock time: %d", n->io_blocks_left);
        wq.push_wait(&wq, n);
      }
    }
  }

  // wait for things to go down
  pthread_t waiting;
  pthread_t ready;
  if (pthread_create(&waiting, NULL, &stcfHold, NULL)){
    printf("Could not create thread \n");
  }
  if (pthread_create(&ready, NULL, &stcfReady, NULL)) {
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
