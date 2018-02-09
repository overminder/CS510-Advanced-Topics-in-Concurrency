#include "tests.h"
#include "util.h"
#include "worker.h"

#define TI_ALLOC(tn, ty, nptr) \
  uint64_t tn; ty nptr; \
  pool_allocate(d, &tn); \
  nptr = (ty) get_qptr(tn)

void announce(volatile uint64_t *p, uint64_t phase, int t_i)
{
  _CMM_STORE_SHARED(*p, phase);

  cmm_smp_mb(); // necessary?

  printf_worker("%d: announced finished with phase %" PRIu64 "\n", t_i, phase);
}

void announce_then_spin(volatile uint64_t *p, uint64_t phase, int t_i)
{
  announce(p, phase, t_i);
  spin_until(&completed_phase, phase, t_i);
}

void* ti(void *data) {
  int                i, j, m, m_pool_lines, tryret, ret, run, nt, si, ei, test, accesses;
  op                 *cur, *start, *end;
  ti_data_in         *d = (ti_data_in*)data;
  pthread_spinlock_t pthreads_spinlock_copy;
  uint64_t           my_spinlock_copy;
  volatile uint64_t  my_place;
  op                 op_copy;
  mcs_sharing        my_mcs_sharing_copy;
  mcs_nosharing      my_mcs_nosharing_copy;
  ticket_state       my_ticket_lock_copy;
  coarse_queue       my_coarse_queue_copy;
  nb_queue           my_nb_queue_copy;
  uint64_t           k, v, enq_op, deq_op, no_deq_op;
  
  // get this thread running on its own CPU
  set_affinity();

  for(test=0; test<n_tests; ++test) {
    if(!test_on[test]) { continue; }
    for(run=0; run<n_runs; ++run) {
      // initialize test data
      m   = n_ops / n_threads;
      end = ( d->i == n_threads - 1 ? oss[test] + n_ops : oss[test] + (d->i + 1) * m);
      switch (test) {
        case SPIN_TRY_LOCK_CORRECTNESS_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = SPIN_TRY_LOCK_INC_OP }));
					}
          break;
        case SPIN_LOCK_CORRECTNESS_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = SPIN_LOCK_INC_OP }));
					}
          break;
        case SPIN_WAIT_LOCK_CORRECTNESS_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = SPIN_WAIT_LOCK_INC_OP }));
					}
          break;
        case SPIN_READ_LOCK_CORRECTNESS_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = SPIN_READ_LOCK_INC_OP }));
					}
          break;
        case SPIN_EXPERIMENTAL_LOCK_CORRECTNESS_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = SPIN_EXPERIMENTAL_LOCK_INC_OP }));
					}
          break;
        case ABQL_SHARING_CORRECTNESS_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = ABQL_SHARING_INC_OP }));
					}
          break;
        case ABQL_NOSHARING_CORRECTNESS_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = ABQL_NOSHARING_INC_OP }));
					}
          break;
        case MCS_SHARING_CORRECTNESS_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = MCS_SHARING_INC_OP }));
					}
          break;
        case MCS_NOSHARING_CORRECTNESS_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = MCS_NOSHARING_INC_OP }));
					}
          break;
        case TICKET_CORRECTNESS_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = TICKET_INC_OP }));
					}
          break;
        case PTHREAD_SPIN_LOCK_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = PTHREAD_SPIN_LOCK_LOCK_OP }));
					}
          break;
        case SPIN_TRY_LOCK_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = SPIN_TRY_LOCK_LOCK_OP }));
					}
          break;
        case SPIN_LOCK_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = SPIN_LOCK_LOCK_OP }));
					}
          break;
        case SPIN_WAIT_LOCK_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = SPIN_WAIT_LOCK_LOCK_OP }));
					}
          break;
        case SPIN_READ_LOCK_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = SPIN_READ_LOCK_LOCK_OP }));
					}
          break;
        case SPIN_EXPERIMENTAL_LOCK_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = SPIN_EXPERIMENTAL_LOCK_LOCK_OP }));
					}
          break;
        case ABQL_SHARING_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = ABQL_SHARING_LOCK_OP }));
					}
          break;
        case ABQL_NOSHARING_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = ABQL_NOSHARING_LOCK_OP }));
					}
          break;
        case MCS_SHARING_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = MCS_SHARING_LOCK_OP }));
					}
          break;
        case MCS_NOSHARING_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = MCS_NOSHARING_LOCK_OP }));
					}
          break;
        case TICKET_TEST:
					for(cur = oss[test] + d->i * m; cur<end; ++cur) {
						_CMM_STORE_SHARED(*cur, ((op){ .operation = TICKET_LOCK_OP }));
					}
          break;
        case COARSE_QUEUE_CORRECTNESS_TEST:
        case COARSE_QUEUE_TEST:
        case COARSE_QUEUE_ENQ_TEST:
        case NB_QUEUE_CORRECTNESS_TEST:
        case NB_QUEUE_TEST:
        case NB_QUEUE_ENQ_TEST:
          switch (test) {
            case COARSE_QUEUE_CORRECTNESS_TEST:
            case COARSE_QUEUE_TEST:
            case COARSE_QUEUE_ENQ_TEST:
              enq_op = COARSE_QUEUE_ENQ_OP;
              break;
            case NB_QUEUE_CORRECTNESS_TEST:
            case NB_QUEUE_TEST:
            case NB_QUEUE_ENQ_TEST:
              enq_op = NB_QUEUE_ENQ_OP;
              break;
          }
          no_deq_op = 0;
          switch (test) {
            case COARSE_QUEUE_CORRECTNESS_TEST:
              deq_op = COARSE_QUEUE_DEQ_REC_OP;
              break;
            case COARSE_QUEUE_TEST:
              deq_op = COARSE_QUEUE_DEQ_OP;
              break;
            case NB_QUEUE_CORRECTNESS_TEST:
              deq_op = NB_QUEUE_DEQ_REC_OP;
              break;
            case NB_QUEUE_TEST:
              deq_op = NB_QUEUE_DEQ_OP;
              break;
            case NB_QUEUE_ENQ_TEST:
            case COARSE_QUEUE_ENQ_TEST:
              no_deq_op = 1;
              break;
          }
          v = m*d->i + 2; // workers enqueue disjoint sets of keys
          d->n_enqueues = 0;
          for(cur = oss[test] + d->i * m; cur<end; ++cur) {
            lrand48_r(&d->rand_state, (long int*)&k);
            if(k % 2 == 0 || no_deq_op) {
              // enqueue when k is even
              _CMM_STORE_SHARED(*cur, ((op){ .operation = enq_op
                                           , .params    = { v++ } })); 
              ++(d->n_enqueues);
            }
            else {
              // dequeue for each odd if applicable
              _CMM_STORE_SHARED(*cur, ((op){ .operation = deq_op })); 
            }
          }
          break;
        default: 
          printf_error("Undefined test %d\n", test);
          exit(-1);
          break;
      }
      // let the parent know we're done and wait for parent to say that everyone
      // else is done
      announce_then_spin(&d->phase, READY_PHASE, d->i);

      // run the test for each number of threads
      for(nt=1; nt<=n_threads; ++nt) {
        for(accesses=0; accesses < (accesses_on[test] ? n_accesses : 1); ++accesses) {
          if(d->i > nt - 1) {
            // this worker is inactive for this nt, just hang out without consuming
            // a lot of resources

            printf_worker("%d: nt is %d, skipping this round\n", d->i, nt);

            // no warming to do, say I'm done and wait for parent to say that everyone
            // else is done
            announce(&d->phase, WARMED_PHASE, d->i);
            //sleep_until(&completed_phase, WARMED_PHASE, d->i);
            spin_until(&completed_phase, WARMED_PHASE, d->i);

            // no ops to do, say I'm done and wait for parent to say that everyone
            // else is done
            announce(&d->phase, OPS_PHASE, d->i);
            //sleep_until(&completed_phase, OPS_PHASE, d->i);
            spin_until(&completed_phase, OPS_PHASE, d->i);

            // similarly, opt out of the checking phase
            announce(&d->phase, CHECK_PHASE, d->i);
            spin_until(&completed_phase, CHECK_PHASE, d->i);
          }
          else {
            // this worker is active for this nt, let's run some tests!

            // per-test-run initialization
                       m               = n_ops / nt;
            uint64_t   m_pool_lines    = n_pool_lines / nt;
                       si              = d->i * m;
            uint64_t   si_pool         = d->i * m_pool_lines;
                       start           = oss[test] + si;
            pool_meta *start_pool_meta = global_pool_meta + si_pool;
            uint64_t  *start_pool_data = global_pool_data + si_pool;
                       ei              = si + ( d->i == nt - 1 ? n_ops - si : m ) - 1;
            uint64_t   ei_pool         = si_pool + ( d->i == nt - 1 ? n_pool_lines - si_pool : m_pool_lines ) - 1;
                       end             = oss[test] + ei;
            pool_meta *end_pool_meta   = global_pool_meta + ei_pool;
            uint64_t  *end_pool_data   = global_pool_data + ei_pool;

            // initialize memory pool if needed
            switch (test) {
              case COARSE_QUEUE_CORRECTNESS_TEST:
              case COARSE_QUEUE_TEST:
              case COARSE_QUEUE_ENQ_TEST:
              case NB_QUEUE_CORRECTNESS_TEST:
              case NB_QUEUE_TEST:
              case NB_QUEUE_ENQ_TEST:
                // initialize memory pool state
                d->pool_cur   = 0;
                d->pool_lines = m_pool_lines + (d->i == nt - 1 ? n_pool_lines % m_pool_lines : 0);
                d->pool_meta  = global_pool_meta + d->i * m_pool_lines;
                d->pool_data  = global_pool_data + d->i * m_pool_lines * CACHE_LINE;
                if((uint64_t)d->pool_data % CACHE_LINE_BYTES != 0) {
                  printf_error("%d: local pool %" PRIx64 " isn't aligned, "
                               "previous cache line boundary is %" PRIx64 "\n"
                               , d->i
                               , (uint64_t)d->pool_data
                               , (uint64_t)d->pool_data % CACHE_LINE_BYTES);
                  while(1);
                }
                for(i=si_pool; i<=ei_pool; ++i) {
                   // clear metadata
                   _CMM_STORE_SHARED(global_pool_meta[i].status, FREE); 
                   _CMM_STORE_SHARED(global_pool_meta[i].sequence_number, 0);
                   // zero data
                   for(j=0; j<CACHE_LINE; ++j) {
                     _CMM_STORE_SHARED(global_pool_data[i*CACHE_LINE + j], 0);
                   }
                }
              break;
            }
            
            // initialize checking state if needed
            switch (test) {
              case COARSE_QUEUE_CORRECTNESS_TEST:
              case NB_QUEUE_CORRECTNESS_TEST:
                // initialize dequeue checking state
                d->dequeues   = global_dequeues + d->i * m;
                d->n_dequeues = 0;
                // shouldn't need max_dequeues, can't have more dequeues than ops
                for(i=0; i<nt; ++i) { d->ti_dequeues_curs[i] = 0; }
              break;
            }

            // worker contribution to data structure initialization
            switch(test) {
              case NB_QUEUE_CORRECTNESS_TEST:
              case NB_QUEUE_TEST:
              case NB_QUEUE_ENQ_TEST:
                // I've set up the memory allocator so that only workers can
                // use it.  This should arguably happen in tests.c, but because
                // it requires memory allocation, we'll just make the first
                // worker do it.
                if(d->i == 0) {
                  // TODO allocate and initialize new dummy node for nb_queue
                  TI_ALLOC(new_node, coarse_node *, new_node_ptr);
                  new_node_ptr->next = 0;
                  new_node_ptr->val = 1;
                  my_nb_queue.head = my_nb_queue.tail = new_node;
                }
                break;
            }

            printf_worker("%d: nt is %d, running ops %d-%d [0x%" PRIx64 "-0x%" PRIx64 "]\n", d->i, nt, si, ei, (uint64_t)start, (uint64_t)end);

            // warm the cache
            // TODO does warming the ops help?
            for(cur = start; cur <= end; ++cur) { op_copy = *cur; }

            switch (test) {
              case PTHREAD_SPIN_LOCK_TEST:
				  			pthreads_spinlock_copy = pthreads_spinlock;
                break;
              case SPIN_TRY_LOCK_CORRECTNESS_TEST:
              case SPIN_LOCK_CORRECTNESS_TEST:
              case SPIN_WAIT_LOCK_CORRECTNESS_TEST:
              case SPIN_READ_LOCK_CORRECTNESS_TEST:
              case SPIN_EXPERIMENTAL_LOCK_CORRECTNESS_TEST:
              case SPIN_TRY_LOCK_TEST:
              case SPIN_LOCK_TEST:
              case SPIN_WAIT_LOCK_TEST:
              case SPIN_READ_LOCK_TEST:
              case SPIN_EXPERIMENTAL_LOCK_TEST:
                my_spinlock_copy = my_spinlock;
                break;
              case ABQL_SHARING_CORRECTNESS_TEST:
              case ABQL_SHARING_TEST:
                for(i=0; i<n_threads; ++i) { my_spinlock_copy = flags_sharing[i].val; }
                break;
              case ABQL_NOSHARING_CORRECTNESS_TEST:
              case ABQL_NOSHARING_TEST:
                for(i=0; i<n_threads; ++i) { my_spinlock_copy = flags_nosharing[i].val; }
                break;
              case MCS_SHARING_CORRECTNESS_TEST:
              case MCS_SHARING_TEST:
                for(i=0; i<n_threads; ++i) { my_mcs_sharing_copy = mcss_sharing[i]; }
                break;
              case MCS_NOSHARING_CORRECTNESS_TEST:
              case MCS_NOSHARING_TEST:
                for(i=0; i<n_threads; ++i) { my_mcs_nosharing_copy = mcss_nosharing[i]; }
                break;
              case TICKET_CORRECTNESS_TEST:
              case TICKET_TEST:
                my_ticket_lock_copy = my_ticket_lock;
                break;
              case COARSE_QUEUE_CORRECTNESS_TEST:
              case COARSE_QUEUE_TEST:
              case COARSE_QUEUE_ENQ_TEST:
                my_coarse_queue_copy = my_coarse_queue;
                break;
              case NB_QUEUE_CORRECTNESS_TEST:
              case NB_QUEUE_TEST:
              case NB_QUEUE_ENQ_TEST:
                my_nb_queue_copy = my_nb_queue;
                break;
              default:
				  			printf_error("Undefined test %d\n", test);
                exit(-1);
				  			break;
            }

            cmm_smp_mb(); // make sure warming loads happen before announcement

            announce_then_spin(&d->phase, WARMED_PHASE, d->i);

            // run the ops!
            for(cur = start, i=si; cur <= end; ++cur, ++i) {
              printf_worker("%d: running op %d\n", d->i, i);

              // enter critical section
              switch (cur->operation) {
                case PTHREAD_SPIN_LOCK_LOCK_OP:
                  pthread_spin_lock(&pthreads_spinlock);
                  break;
                case SPIN_TRY_LOCK_LOCK_OP:
                case SPIN_TRY_LOCK_INC_OP:
                  tryret = spin_try_lock(&my_spinlock);
                  break;
                case SPIN_LOCK_LOCK_OP:
                case SPIN_LOCK_INC_OP:
                  spin_lock(&my_spinlock);
                  break;
                case SPIN_WAIT_LOCK_LOCK_OP:
                case SPIN_WAIT_LOCK_INC_OP:
                  spin_wait_lock(&my_spinlock);
                  break;
                case SPIN_READ_LOCK_LOCK_OP:
                case SPIN_READ_LOCK_INC_OP:
                  spin_read_lock(&my_spinlock);
                  break;
                case SPIN_EXPERIMENTAL_LOCK_LOCK_OP:
                case SPIN_EXPERIMENTAL_LOCK_INC_OP:
                  spin_experimental_lock(&my_spinlock);
                  break;
                case ABQL_SHARING_LOCK_OP:
                case ABQL_SHARING_INC_OP:
                  abql_sharing_lock(&my_place, &queue_last_sharing, flags_sharing, n_threads);
                  break;
                case ABQL_NOSHARING_LOCK_OP:
                case ABQL_NOSHARING_INC_OP:
                  abql_nosharing_lock(&my_place, &queue_last_nosharing, flags_nosharing, n_threads);
                  break;
                case MCS_SHARING_LOCK_OP:
                case MCS_SHARING_INC_OP:
                  mcs_sharing_lock(&mcs_global_sharing, &mcss_sharing[d->i]);
                  break;
                case MCS_NOSHARING_LOCK_OP:
                case MCS_NOSHARING_INC_OP:
                  mcs_nosharing_lock(&mcs_global_nosharing, &mcss_nosharing[d->i]);
                  break;
                case TICKET_LOCK_OP:
                case TICKET_INC_OP:
                  ticket_lock(&my_ticket_lock);
                  break;
                case COARSE_QUEUE_ENQ_OP:
                case NB_QUEUE_ENQ_OP:
                case COARSE_QUEUE_DEQ_OP:
                case NB_QUEUE_DEQ_OP:
                case COARSE_QUEUE_DEQ_REC_OP:
                case NB_QUEUE_DEQ_REC_OP:
                  // these operations don't use locks, critical section is just the body
                  break;
                default:
                  printf_error("Undefined operation %" PRIu64 "\n", cur->operation);
                  exit(-1);
                  break;
              }

              // critical section body

              // * simulate work by accessing memory
              if(accesses_on[test]) { access(i, d, accessesv[accesses]); }

              // * do per-operation critical section work
              switch(cur->operation) {
                case SPIN_LOCK_INC_OP:
                case SPIN_WAIT_LOCK_INC_OP:
                case SPIN_READ_LOCK_INC_OP:
                case SPIN_EXPERIMENTAL_LOCK_INC_OP:
                case ABQL_SHARING_INC_OP:
                case ABQL_NOSHARING_INC_OP:
                case MCS_SHARING_INC_OP:
                case MCS_NOSHARING_INC_OP:
                case TICKET_INC_OP:
                  ++my_spinlock_shared_counter;
                  ++d->my_spinlock_counter;
                  break;
                case SPIN_TRY_LOCK_INC_OP:
                  if (tryret) {
                    ++my_spinlock_shared_counter;
                    ++d->my_spinlock_counter;
                  }
                  break;
                case COARSE_QUEUE_ENQ_OP:
                  coarse_enqueue(d, &my_coarse_queue, cur->params[0]);
                  break;
                case NB_QUEUE_ENQ_OP:
                  nb_enqueue(d, &my_nb_queue, cur->params[0]);
                  break;
                case COARSE_QUEUE_DEQ_OP:
                  coarse_dequeue(d, &my_coarse_queue, &cur->params[0]);
                  break;
                case NB_QUEUE_DEQ_OP:
                  nb_dequeue(d, &my_nb_queue, &cur->params[0]);
                  break;
                case COARSE_QUEUE_DEQ_REC_OP:
                  if(coarse_dequeue(d, &my_coarse_queue, d->dequeues + d->n_dequeues)) {
                    ++(d->n_dequeues);
                  }
                  break;
                case NB_QUEUE_DEQ_REC_OP:
                  if(nb_dequeue(d, &my_nb_queue, d->dequeues + d->n_dequeues)) {
                    ++(d->n_dequeues);
                  }
                  break;
              }

              // leave critical section
              switch (cur->operation) {
                case PTHREAD_SPIN_LOCK_LOCK_OP:
                  pthread_spin_unlock(&pthreads_spinlock);
                  break;
                case SPIN_LOCK_LOCK_OP:
                case SPIN_LOCK_INC_OP:
                case SPIN_WAIT_LOCK_LOCK_OP:
                case SPIN_WAIT_LOCK_INC_OP:
                case SPIN_READ_LOCK_LOCK_OP:
                case SPIN_READ_LOCK_INC_OP:
                case SPIN_EXPERIMENTAL_LOCK_LOCK_OP:
                case SPIN_EXPERIMENTAL_LOCK_INC_OP:
                  spin_unlock(&my_spinlock);
                  break;
                case SPIN_TRY_LOCK_LOCK_OP:
                case SPIN_TRY_LOCK_INC_OP:
                  if(tryret) { spin_unlock(&my_spinlock); }
                  break;
                case ABQL_SHARING_LOCK_OP:
                case ABQL_SHARING_INC_OP:
                  abql_sharing_unlock(&my_place, flags_sharing, n_threads);
                  break;
                case ABQL_NOSHARING_LOCK_OP:
                case ABQL_NOSHARING_INC_OP:
                  abql_nosharing_unlock(&my_place, flags_nosharing, n_threads);
                  break;
                case MCS_SHARING_LOCK_OP:
                case MCS_SHARING_INC_OP:
                  mcs_sharing_unlock(&mcs_global_sharing, &mcss_sharing[d->i]);
                  break;
                case MCS_NOSHARING_LOCK_OP:
                case MCS_NOSHARING_INC_OP:
                  mcs_nosharing_unlock(&mcs_global_nosharing, &mcss_nosharing[d->i]);
                  break;
                case TICKET_LOCK_OP:
                case TICKET_INC_OP:
                  ticket_unlock(&my_ticket_lock);
                  break;
              }

            }

            // I'm done with my ops, say so and wait for parent to say that
            // everyone else is done
            announce_then_spin(&d->phase, OPS_PHASE, d->i);

            // do per-thread checking passes if applicable
            switch(test) {
              case COARSE_QUEUE_CORRECTNESS_TEST:
              case NB_QUEUE_CORRECTNESS_TEST:
                for(cur = start; cur <= end; ++cur) {
                  switch(cur->operation) {
                    case COARSE_QUEUE_CORRECTNESS_TEST:
                      enq_op = COARSE_QUEUE_ENQ_OP;
                      break;
                    case NB_QUEUE_CORRECTNESS_TEST:
                      enq_op = NB_QUEUE_ENQ_OP;
                      break;
                  }
                  // TODO check that the nodes that remain enqueued at the end
                  // are FIFO.
                  if(cur->operation == enq_op) {
                    k = cur->params[0];
                    for(i=0; i<nt; ++i) {
                      for(j=0; j<global_datas[i].n_dequeues; ++j) {
                        if(global_datas[i].dequeues[j] == k) {
                          if(j < d->ti_dequeues_curs[i]) {
                            printf_error("%d: dequeued %" PRIu64 " before "
                                         "earlier enqueue %" PRIu64 
                                         ", %s is not FIFO from my perspective.\n"
                                         , d->i, k
                                         , global_datas[i].dequeues[d->ti_dequeues_curs[i]]
                                         , test_names[test]);
                            while(1);
                            exit(-1);
                          }
                          else {
                            d->ti_dequeues_curs[i] = j;
                          }
                        }
                      }
                    }
                  }
                }
                break;
            }

            announce_then_spin(&d->phase, CHECK_PHASE, d->i);
          }
        }
      }
    }
    announce_then_spin(&d->phase, INIT_PHASE, d->i);
  }

  return data;
}

// spin_try_lock
int spin_try_lock(volatile uint64_t *lock) {
  if (lockcmpxchgq(lock, UNLOCKED, LOCKED) == UNLOCKED) {
    // *lock == UNLOCKED: success
    return 0;
  } else {
    return 1;
  }
}

// spin_lock
void spin_lock(volatile uint64_t *lock) {
  while (spin_try_lock(lock) != 0);
}

// spin_wait_lock
void spin_wait_lock(volatile uint64_t *lock) {
  while (spin_try_lock(lock) != 0) {
    volatile int i = 500;
    while (i > 0) {
      --i;
    }
  }
}

// spin_read_lock
void spin_read_lock(volatile uint64_t *lock) {
  // TODO
  do {
    while (*lock == LOCKED);
  } while (spin_try_lock(lock) != 0);
}

// spin_experimental_lock
void spin_experimental_lock(volatile uint64_t *lock) {
  // TODO
}
    

void spin_unlock(volatile uint64_t *lock) {
  // TODO
  *lock = UNLOCKED;
}

// BEGIN ASSIGNMENT SECTION

void ticket_lock(volatile ticket_state *lock) {
  uint64_t my_ticket = lockxaddq(&lock->next, 1);
  while (_CMM_LOAD_SHARED(lock->owner) != my_ticket);
}

void ticket_unlock(volatile ticket_state *lock) {
  lockxaddq(&lock->owner, 1);
}

void abql_sharing_lock(volatile uint64_t *my_place, volatile uint64_t *queue_last, 
                       volatile flag_sharing *flags, uint64_t n_threads) {
  // TODO
  uint64_t me = lockxaddq(queue_last, 1);
  // my_place is only used locally so a fence would be useless.
  *my_place = me;
  while (_CMM_LOAD_SHARED(flags[me % n_threads].val) != HAS_LOCK);
}

void abql_nosharing_lock(volatile uint64_t *my_place, volatile uint64_t *queue_last, 
                         volatile flag_nosharing *flags, uint64_t n_threads) {
  // TODO
  uint64_t me = lockxaddq(queue_last, 1);
  // my_place is only used locally so a fence would be useless.
  *my_place = me;
  while (_CMM_LOAD_SHARED(flags[me % n_threads].val) != HAS_LOCK);
}

void abql_sharing_unlock(volatile uint64_t *my_place, volatile flag_sharing *flags, uint64_t n_threads) {
  // TODO
  uint64_t me = *my_place;
  _CMM_STORE_SHARED(flags[(me + 1) % n_threads].val, HAS_LOCK);
  flags[me % n_threads].val = MUST_WAIT;
}

void abql_nosharing_unlock(volatile uint64_t *my_place, volatile flag_nosharing *flags, uint64_t n_threads) {
  // TODO
  uint64_t me = *my_place;
  _CMM_STORE_SHARED(flags[(me + 1) % n_threads].val, HAS_LOCK);
  // _CMM_STORE_SHARED(flags[me % n_threads].val, MUST_WAIT);
  flags[me % n_threads].val = MUST_WAIT;
}

void mcs_sharing_lock(volatile mcs_sharing *global_lock, volatile mcs_sharing *local_lock) {
  _CMM_STORE_SHARED(local_lock->next, 0);
  // TODO
  volatile mcs_sharing *owner = (void *) xchgq(&global_lock->next, (uintptr_t) local_lock);
  if (owner) {
    _CMM_STORE_SHARED(local_lock->locked, LOCKED);
    _CMM_STORE_SHARED(owner->next, (uintptr_t) local_lock);
    while (_CMM_LOAD_SHARED(local_lock->locked) == LOCKED);
  } else {
    // Acquired
  }
}

void mcs_nosharing_lock(volatile mcs_nosharing *global_lock, volatile mcs_nosharing *local_lock) {
  // TODO
  _CMM_STORE_SHARED(local_lock->next, 0);
  // TODO
  volatile mcs_nosharing *owner = (void *) xchgq(&global_lock->next, (uintptr_t) local_lock);
  if (owner) {
    _CMM_STORE_SHARED(local_lock->locked, LOCKED);
    _CMM_STORE_SHARED(owner->next, (uintptr_t) local_lock);
    while (_CMM_LOAD_SHARED(local_lock->locked) == LOCKED);
  } else {
    // Acquired
  }
}

void mcs_sharing_unlock(volatile mcs_sharing *global_lock, volatile mcs_sharing *local_lock) {
  volatile mcs_sharing *next;
  // Crucial to not do the lockcmpxchg first to avoid more invalidation.
  if (next = _CMM_LOAD_SHARED(*(mcs_sharing **) &local_lock->next)) {
  } else {
    if (lockcmpxchgq(&global_lock->next, (uintptr_t) local_lock, 0) ==
        (uintptr_t) local_lock) {
      // Done.
      return;
    } else {
      // Someone is waiting.
      while (!(next = _CMM_LOAD_SHARED(*(mcs_sharing **) &local_lock->next)));
    }
  }
  _CMM_STORE_SHARED(next->locked, UNLOCKED);
}

void mcs_nosharing_unlock(volatile mcs_nosharing *global_lock, volatile mcs_nosharing *local_lock) 
{
  // TODO
  volatile mcs_nosharing *next;
  // Crucial to not do the lockcmpxchg first to avoid more invalidation.
  if (next = _CMM_LOAD_SHARED(*(mcs_nosharing **) &local_lock->next)) {
  } else {
    if (lockcmpxchgq(&global_lock->next, (uintptr_t) local_lock, 0) ==
        (uintptr_t) local_lock) {
      // Done.
      return;
    } else {
      // Someone is waiting.
      while (!(next = _CMM_LOAD_SHARED(*(mcs_nosharing **) &local_lock->next)));
    }
  }
  _CMM_STORE_SHARED(next->locked, UNLOCKED);
}

// BEGIN ASSIGNMENT SECTION

#define WITH_QUEUE_LOCK(x) { \
  spin_read_lock(&queue->lock); \
  x \
  spin_unlock(&queue->lock); \
}

void coarse_enqueue(ti_data_in *d, volatile coarse_queue *queue, uint64_t value) { 
  TI_ALLOC(tagged, coarse_node *, nptr);
  _CMM_STORE_SHARED(nptr->val, value);
  _CMM_STORE_SHARED(nptr->next, 0);

  WITH_QUEUE_LOCK({
      if (!_CMM_LOAD_SHARED(queue->head)) {
        // empty -> 1
        _CMM_STORE_SHARED(queue->head, tagged);
      } else {
        // N -> N+1
        coarse_node *tptr = (coarse_node *) get_qptr(_CMM_LOAD_SHARED(queue->tail));
        _CMM_STORE_SHARED(tptr->next, tagged);
      }
      _CMM_STORE_SHARED(queue->tail, tagged);
  });
}

int coarse_dequeue(ti_data_in *d, volatile coarse_queue *queue, uint64_t *ret) { 
  int res;
  coarse_node *hptr = NULL;
  WITH_QUEUE_LOCK({
      uintptr_t head = CMM_LOAD_SHARED(queue->head);
      uintptr_t tail = CMM_LOAD_SHARED(queue->tail);

      if (!head) {
        // empty
        res = 0;
      } else {
        // non-empty
        res = 1;
        hptr = (coarse_node *) get_qptr(head);
        *ret = CMM_LOAD_SHARED(hptr->val);
        uint64_t next = CMM_LOAD_SHARED(hptr->next);
        if (head == tail) {
          // 1 -> 0
          CMM_STORE_SHARED(queue->head, 0);
          CMM_STORE_SHARED(queue->tail, 0);
        } else {
          // N+1 -> N
          CMM_STORE_SHARED(queue->head, next);
        }
      }
  });
  if (hptr) {
    pool_free(d, hptr);
  }
  return res;
}

#define NB_CHASE() \
    lockcmpxchgq(&queue->tail, tail, next)

void nb_enqueue(ti_data_in *d, volatile nb_queue *queue, uint64_t value) {
  TI_ALLOC(n, nb_node *, nptr);
  CMM_STORE_SHARED(nptr->val, value);
  CMM_STORE_SHARED(nptr->next, 0);

  uint64_t tail, next;

  while (true) {
    tail = CMM_LOAD_SHARED(queue->tail);
    assert(tail != 0 && "tail is null?");
    nb_node *tail_ptr = (nb_node *) get_qptr(tail);
    next = CMM_LOAD_SHARED(tail_ptr->next);

    // if (tail == CMM_LOAD_SHARED(queue->tail)) {
      if (next == 0) {
        if (lockcmpxchgq(&tail_ptr->next, next, n) == next) {
          break;
        }
      } else {
        // Some one enqueued before us: chase
        NB_CHASE();
      }
    // }
  }
  // We just mutated tail->next: chase
  lockcmpxchgq(&queue->tail, tail, n);
}

int nb_dequeue(ti_data_in *d, volatile nb_queue *queue, uint64_t *ret) {
  uint64_t head, tail, next;
  nb_node *head_ptr;

  while (true) {
    head = CMM_LOAD_SHARED(queue->head);
    tail = CMM_LOAD_SHARED(queue->tail);
    assert(head != 0 && "head is null?");
    assert(tail != 0 && "tail is null?");
    head_ptr = (nb_node *) get_qptr(head);
    next = CMM_LOAD_SHARED(head_ptr->next);
    nb_node *next_ptr = (nb_node *) get_qptr(next);

    // if (head == CMM_LOAD_SHARED(queue->head)) {
      if (head == tail) {
        if (!next) {
          // Empty.
          return 0;
        }
        // Falling behind.
        NB_CHASE();
      } else {
        *ret = CMM_LOAD_SHARED(next_ptr->val);
        if (lockcmpxchgq(&queue->head, head, next) == head) {
          break;
        }
      }
    // }
  }
  pool_free(d, head_ptr);
  return 1;
}

// END ASSIGNMENT SECTION

