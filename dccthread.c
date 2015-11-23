/* By Thiago Silva */

#include "dccthread.h"


void dccthread_init(void (*func)(int), int param)
{
  THREAD_QUEUE_INITIAL_POS=THREAD_QUEUE_FINAL_POS=0;
  thread_ready_queue = (dccthread_t *)malloc(THREAD_QUEUE_SIZE*sizeof(dccthread_t));
  
  
}

dccthread_t * dccthread_create(const char *name, void (*func)(int ), int param)
{
  
}

void dccthread_yield(void)
{
  
}

void dccthread_exit(void)
{}  

void dccthread_wait(dccthread_t *tid)
{}

void dccthread_sleep(struct timespec ts)
{}

dccthread_t * dccthread_self(void)
{}

const char *dccthread_name(dccthread_t *tid)
{}


/* Just returns 1 if the queue is empty or 0 otherwise. */
inline int is_thread_queue_empty()
{
  return THREAD_QUEUE_INITIAL_POS == THREAD_QUEUE_FINAL_POS;
}

/* This function returns 1 if the queue is full. */
inline int is_thread_queue_full()
{
  return THREAD_QUEUE_INITIAL_POS == (THREAD_QUEUE_FINAL_POS+1)%THREAD_QUEUE_SIZE;
}
