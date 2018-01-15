### Benchmarking your `spin_lock()` implementation

1. Does calling `spin_try_lock(&my_spinlock)` over and over again (even after
   we've just observed that `my_spinlock` is held) every time we need to get
   `my_spinlock` affect how long it takes to finish a set of operations?  Does
   each call require threads to communicate, and if so, what shared resources
   does that communication use?

A: Yes, calling `spin_try_lock` is costy. Yes, each call require threads
   to communicate. The bus is used as the shared resource for communication.
   [Section 8.1.4 from the manual](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.pdf) says 
   that modern (P6 and after) Intel CPU, when executing LOCK prefixed instructions,
   will almost always lock the bus to ensure sequential consistency. The
   only case when it's not locked is when there's no contention (i.e.,
   the CPU has exclusive ownership on the cache line being locked).

2. How does this implementation scale as we increase the number of threads?
   Does passing the lock around and busy-waiting for it affect the time it
   takes for all threads to crunch through the set of operations?  If so, is the
   effect different for different numbers of threads?

A: Scales really bad. Yes, the more contention, the worse performance.

3. As we add bus traffic, first by updating a single cache line and then by
   updating 10 cache lines while holding the lock, how do `spin_lock` and
   `pthread_spin_lock` respond to this change, and why?

A: Both become even slower as we will be invalidating more cache lines
   so there's more bus contention.

### `spin_wait_lock()`

4. What result did you expect?  Are these graphs consistent with your expectation?

A: I expect `spin_wait_lock()` to be more performant than the naive
   `spin_lock()` implementation. Yes.

5. Does `spin_wait_lock()` result in more or less communication than
   `spin_lock()`?  Do the results shed any light on the relative merits of more
   and less communication?

A: Less communication as `spin_wait_lock()` does a bit of local spinning
   before each atomic CAS. In this way, we decrease the frequency of
   cache invalidation (and main memory traffic).

6. Communication always comes at a cost, so we need to think carefully about how
   much bus traffic our implementations generate.  How can we minimize communication in
   our spinlock implementations?  Is there more than one way?

A: Another way is to do a read before the CAS. The read will not invalidate the
   cache line and thus would be faster. Can't think of more ways...

7. If we have just observed `*lock == LOCKED`, how likely is it that
   `spin_try_lock()` will succeed (returning `1`)?  Do you think that
   `spin_try_lock(lock)`, which can trigger an update, requires the same
   amount of communication as just loading the value of `my_spinlock`?  Why?  Do
   you think that these operations require unidirectional or bidirectional
   communication, and why?

A: Very unlikely that it's going to succeed. No, `spin_try_lock()` invalidates
   the cache line where `lock` resides (which will cause other CPUs that are also spinning on this
   lock to read from the main memory on their next CAS instruction),
   while a read will just read from the cache and does nothing else.

  The CAS instruction does a bidirectional communication (does read and write
  in the same instruction) while a simple read only does a unidirectional
  communication.

### `spin_read_lock()`

8. What result did you expect?  Are these graphs consistent with your expectation?

A: Faster than `spin_wait_lock()`. Yes.

9. Does `spin_try_lock(lock)` always return `1` after we observe `*lock ==
   UNLOCKED`?  Why or why not?  If not, what does your implementation do in
   this situation?

A: No since there could be someone that acquire the lock before us. I retry
   the whole read-CAS process in this situation.

10. Do the results suggest that one way of minimizing communication is better
    than the other?

A:  Yes. Polling by simple read instruction before issuing a CAS is the
    best strategy currently.


