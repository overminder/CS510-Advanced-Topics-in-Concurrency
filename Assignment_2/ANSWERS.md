### `ticket_lock()` and `ticket_unlock()`

1. How does `ticket_lock` compare to its unfair counterparts?

A: When the bus traffic is light (no cache line touched in the critical
   section), it's slower than `spin_read_lock` and
   less performant than `spin_lock` when the contention is light and
   more performant when the contention is heavy.

   When the bus traffic is heavy, it's on pair with `spin_read_lock`.

2. In particular, why is there a gap between `spin_lock` and `ticket_lock`?

A: `spin_lock` suffers when the bus traffic is very heavy since each CPU
   spinning on the lock will repeatedly invalidate the cache line that the
   lock belongs to so all other CPUs spinning on it will need to
   read the value again from memory (and thus more bus traffic).

   `ticket_lock` only invalidates the cache line for every CPU when
   when there's an unlock.

3. What memory location(s) are threads contending to update in this implementation?

A: `lock->next`.

4. What memory location(s) are threads spinning on in this implementation?

A: `lock->owner`.

5. What communication (to whom) is triggered when `ticket_unlock` writes to `owner`?

A: Cache invalidation to all CPUs spinning on `lock->owner`.

### `abql_(no)sharing_lock()` and `abql_(no)sharing_unlock()`

6.  How does `abql_sharing_lock` compare to `abql_nosharing_lock`?  Why?

A:  The nosharing version runs faster since it prevents a false sharing
    problem on the flags array and causes the unlock to only invalidate the
    next CPU's cache line (instead of invalidating at most 8 CPUs).

7.  What memory location(s) are threads contending to update in this implementation?

A:  `queue_last`.

8.  What memory location(s) are threads spinning on in this implementation?

A: `flags[*my_place % n_threads]`

9.  What communication (to whom) is triggered when `abql_nosharing_unlock`
    writes to `flags[successor_index].val`?  How does this compare
    to the communication triggered by `ticket_unlock`?

A:  From the current lock holder CPU to the next CPU. It's done in the same
    fashion as `ticket_unlock`.

10. How does `abql_nosharing_lock` compare to `ticket_lock` and why?

A:  ABQL is faster than ticket lock. This is because ticket lock invalidates
    the cache line for all the spinning CPUs on each unlock, while ABQL
    just invalidate the cache line for the next CPU.


