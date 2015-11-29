/* By Thiago Silva */

#include "dccthread.h"


/* Variable that keeps the index of the thread running currently. */
int current_thread;

/* Variables that keep the final position of the queue */
int THREAD_QUEUE_FINAL_POS;

/* Queue that keeps the threads ready to execute */
dccthread_t *thread_ready_queue;

/* The two main threads of the program, initialized at the init function */
ucontext_t manager, principal;

int old_index;



/**************** NOT LISTED ****************/
/* Just returns 1 if the queue is empty or 0 otherwise. */
int is_thread_queue_empty()
{
  return current_thread == THREAD_QUEUE_FINAL_POS;
}

/* This function returns 1 if the queue is full. */
int is_thread_queue_full()
{
  return current_thread == (THREAD_QUEUE_FINAL_POS+1)%THREAD_QUEUE_SIZE;
}

void nextThread()
{
    printf("Running: nextThread()\n");
    while(!is_thread_queue_empty())
    {
      printf("current_thread = %d\tEND = %d\n", current_thread, THREAD_QUEUE_FINAL_POS);
      printf("Swapping Principal -> %s\n", thread_ready_queue[current_thread].name);
      swapcontext(&principal, &thread_ready_queue[current_thread].context);
      printf("Thread %d returned\n", current_thread);
      
      current_thread++;
      current_thread%=THREAD_QUEUE_SIZE;
    }
}

void setStackProperties(ucontext_t *context)
{
  context->uc_stack.ss_sp = malloc(STACK_SIZE);
  context->uc_stack.ss_size = STACK_SIZE;
  context->uc_stack.ss_flags = 0;
}

/**************** END - NOT LISTED ****************/






void dccthread_init(void (*func)(int), int param)
{
  THREAD_QUEUE_FINAL_POS=current_thread=0;
  thread_ready_queue = (dccthread_t *)malloc(THREAD_QUEUE_SIZE*sizeof(dccthread_t*));

  getcontext(&manager);
  getcontext(&principal);
  manager.uc_link = NULL;
  setStackProperties(&manager);
  
  
  principal.uc_link = &manager;
  setStackProperties(&principal);
  makecontext(&principal, nextThread, 1, param);
  dccthread_create("Main", func, param);
  printf("Swapping Manager->principal\n");
  swapcontext(&manager, &principal);
  printf("UE, voltou?\n");
}


dccthread_t * dccthread_create(const char *name, void (*func)(int), int param)
{
  if(is_thread_queue_full())
    return NULL;
  
  dccthread_t neueThread;
  
  ucontext_t neue;
  getcontext(&neue);
  neue.uc_link = &principal;
  setStackProperties(&neue);
  makecontext(&neue, (void (*) (void)) func, 1, param);
  
  neueThread.name = name;
  neueThread.context = neue;
  
  thread_ready_queue[THREAD_QUEUE_FINAL_POS] = neueThread;
  dccthread_t *ret = &thread_ready_queue[THREAD_QUEUE_FINAL_POS];
  
  THREAD_QUEUE_FINAL_POS++;
  THREAD_QUEUE_FINAL_POS%=THREAD_QUEUE_SIZE;
  
  return ret;
}

void dccthread_yield(void)
{
  old_index=THREAD_QUEUE_FINAL_POS;
  thread_ready_queue[old_index] = thread_ready_queue[current_thread];
  THREAD_QUEUE_FINAL_POS++;  THREAD_QUEUE_FINAL_POS%=THREAD_QUEUE_SIZE;
  
  printf("Swapping %d -> Principal\n", current_thread);
  setcontext(&principal);
  //swapcontext(&thread_ready_queue[old_index].context, &principal);
  printf("AHJFLDHJFKDHJKFDHJKFDHJK        %d\n", old_index);
}

void dccthread_exit(void)
{}  

void dccthread_wait(dccthread_t *tid)
{}

void dccthread_sleep(struct timespec ts)
{}

dccthread_t * dccthread_self(void)
{
  // Return the running thread.
  return &thread_ready_queue[current_thread];
}

const char *dccthread_name(dccthread_t *tid)
{
  return tid->name;
}




