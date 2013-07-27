COMP 3500 PROJECT 3
CPU SCHEDULER

Authors:
Josh Acklin,  Reed Allman,  Maghen Smith
jwa0008       rpa0003       mjs0026

Sources:

We compiled and ran this program on tux-184.
All output is from Limception/tracefile
To run:
cd Limception/stcf or Limception/exp and then make
then e.g. ./stcf_scheduler ../tracefile > output (pass tracefile on std_in)

The output of the stcf-p scheduler is in Limception/stcf/stcf_results
The output of the exponential scheduler is in Limception/exp/exp_results

The results and conclusion:

STCF-P wastes significantly less time (about 1/3) and has 25% higher
throughput than the Exponential scheduler. Exponential had a lower minimum
completion time and a higher max completion time. The faster interrupts are
likely to have attributed to this, where exponential is leaving and being ran
more frequently compared to stcf-p. The average completion time was faster for
the exponential scheduler, as well. We think that we would take an exponential
scheduler vs. the stcf-p, because instead of getting locked into a process,
exponential gets updated more frequently. It also seems to be faster, even
with all the extra context switches. 


DESIGN

exp_scheduler.c
1. Exp_scheduler implements the exponential queues version of the project. Processes are placed in a Ready queue when it is their assigned start time, and their time slices are modified according to how much time they take to run. 
2. Nodes hold process information. Nodes hold links to the next node, priority, name, start time, completion time, cpu time, io count, io blocks left, io block time (how long to wait before next io), io block next (keeps track of clock for io), cpu completed (how much cpu time has been completed), and time slice (how many 100ths of a second the process can operate during this slice). Waiting queue holds nodes that are awaiting their proper start time. Nodes are moved to the Ready queue at their start time. The done queue holds nodes for processes that have been completed.
3. 
  Name: push_exponential
  Parameters: Node *n the node to be pushed on the queue
        Queue* q the queue to push the Node on
  Return vals: void
  Description: This function will check if the queue is empty (by using peek). If not empty, we create a node pointer that points to the head of the queue. The function uses a while loop to iterate through the queue and insert the Node *n based on priority. It also increments the size. If the queue is empty, the function sets the head of the queue to the Node *n.

  Name: peek
  Parameters: Queue *q
  Return vals: Node *
  Description: This function returns 0 if the queue is NULL, or 1 if the queue has a head

  Name: pop
  Parameters: Queue *q 
  Return vals: Node *
  Description: This function changes the head of the queue to the second Node in the queue and returns the head of the queue.

  Name: push_wait
  Parameters: Queue *q, Node *n
  Return vals: void
  Description: The function creates a node pointer insert that points to the head of the queue. If the queue is not null, the function will insert the Node *n into the queue according to start time. If the queue is null, the function will insert the Node *n as the head of the queue.

  Name: Run
  Parameters:int clock, Node *n
  Return vals: int value representing the clock value.
  Description: takes a clock  (the clock) and a node to do 'work' on. done is
  an int that's alloted with the current time slice for this node.
  Run will run for this duration or until an IO completes or a new
  process is pushed into the ready queue. If the process completes its time
  slice then it will be alloted a larger time slice, if not, it gets a smaller
  time slice and the priority will go down or up, respectively.
  The time between a process' IO's is
  determined after we push the node to the waiting queue. io_block_next is a
  reference to the amount of time that there is until the next io completes.
  After an IO completes, this value is set back to io_block_time, the interval
  between a process' IO's. An IO runs for 10 seconds if we are at
  io_block_next or if the process has completed all of it's CPU and we have an
  IO. After IO, break will leave (interrupt) this process. IO blocks in the
  process. If the process is done with IO and CPU then we leave without a
  context switch out. This method will return an updated clock that includes
  all of the CPU time that was run on the process.

stcf_scheduler.c
1. Stcf_scheduler implements the shortest time to completion first preemptive algorithm. The Ready queue is sorted according to how much cpu time is remaining in the process, and the current process can be preempted with a new process or a disk IO completion. 
2. Nodes hold process information. Nodes hold links to the next node, priority, name, start time, completion time, cpu time, io count, io blocks left, io block time (how long to wait before next io), io block next (keeps track of clock for io), cpu completed (how much cpu time has been completed), and time slice (how many 100ths of a second the process can operate during this slice). Waiting queue holds nodes that are awaiting their proper start time. Nodes are moved to the Ready queue at their start time. The done queue holds nodes for processes that have been completed.
3.  Name: push_stcf
  Parameters: Node *n the node to be pushed on the queue
        Queue* q the queue to push the Node on
  Return vals: void
  Description: This function will check if the queue is empty (by using peek). If not empty, we create a node pointer that points to the head of the queue. The function uses a while loop to iterate through the queue and insert the Node *n based on cpu time remaining. It also increments the size. If the queue is empty, the function sets the head of the queue to the Node *n.

  Name: peek
  Parameters: Queue *q
  Return vals: Node *
  Description: This function returns 0 if the queue is NULL, or 1 if the queue has a head

  Name: pop
  Parameters: Queue *q 
  Return vals: Node *
  Description: This function changes the head of the queue to the second Node in the queue and returns the head of the queue.

  Name: push_wait
  Parameters: Queue *q, Node *n
  Return vals: void
  Description: The function creates a node pointer insert that points to the head of the queue. If the queue is not null, the function will insert the Node *n into the queue according to start time. If the queue is null, the function will insert the Node *n as the head of the queue.

  Name: Run
  Parameters:int clock, Node *n
  Return vals: int value representing the clock value.
  Description: takes a clock  (the clock) and a node to do 'work' on. done is
  an int that's alloted with the maximum time that this node can run on the
  clock for. Run will run for this duration or until an IO completes or a new
  process is pushed into the ready queue. The time between a process' IO's is
  determined after we push the node to the waiting queue. io_block_next is a
  reference to the amount of time that there is until the next io completes.
  After an IO completes, this value is set back to io_block_time, the interval
  between a process' IO's. An IO runs for 10 seconds if we are at
  io_block_next or if the process has completed all of it's CPU and we have an
  IO. After IO, break will leave (interrupt) this process. IO blocks in the
  process. If the process is done with IO and CPU then we leave without a
  context switch out. This method will return an updated clock that includes
  all of the CPU time that was run on the process. 
