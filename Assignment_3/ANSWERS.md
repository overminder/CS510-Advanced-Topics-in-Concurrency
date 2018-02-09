### Coarse Queue

1. Can an enqueuer and dequeuer concurrently proceed in this implementation?  Why or why not?

A: No, since there's only one shared lock for both enqueuing and dequeuing.

### Non-Blocking Concurrent Queue

2. Can an enqueuer and dequeuer concurrently proceed in this implementation?

A: Yes.

3. How does `nb_queue` compare to `coarse_queue` (i.e. for a mixed workload)?

A: `coarse_queue` goes slower as the number of threads increases.
   `nb_queue` almost stays the same.

4. How does `nb_queue_enq` compare to `coarse_queue_enq` (i.e. for a workload
   of only enqueues)? 

A: `coarse_queue` goes slower as the number of threads increases.
   `nb_queue` also goes slower but stays constant after ~16 threads.

5. What primitives does `nb_enqueue()` use to make updates on a best-case
   enqueue?  What about `coarse_enqueue()`?

A: `nb_enqueue`: 2 CASs: A CAS to set `tail_ptr->next = new_node` and another
   CAS to set `queue->tail = new_node`.

   `coarse_enqueue()`: 1 CAS to lock the spinlock. 

6. Do the implementations compare in the same way for both kinds of workloads?
   Why or why not?

A: Mixed workload is more favorable for `nb_queue` as it allows
   concurrent enqueue and dequeue.

   Pure enqueuing makes the distinction smaller.

7. Where would the ABA problem break this operation?  What could go wrong as a
   result?

   Say thread 1 running the dequeue function is preempted just before
   the second CAS (i.e., between line D12 and D13 in the
   figure 1 in the concurrent queue paper). Then thread 2 enqueues and
   dequeues multiple times to cause our memory pool to wraparound and
   a node with exactly the same cache line and tag is put in `queue->head`.
   Then thread 1's CAS will succeed but the head is actually not the same.

