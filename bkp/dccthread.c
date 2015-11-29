/* By Thiago Silva */

#include "dccthread.h"


/* Variable that keeps the index of the thread running currently. */
int current_thread=0;

/* Variables that keep the final position of the queue */
int THREAD_QUEUE_FINAL_POS=0;

/* Queue that keeps the threads ready to execute */
dccthread_t *thread_ready_queue[THREAD_QUEUE_SIZE];

/* The two main threads of the program, initialized at the init function */
ucontext_t manager, principal;



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
    printf("Empty? %d\n", is_thread_queue_empty());
    if(is_thread_queue_empty()) return;
    current_thread++;
    current_thread%=THREAD_QUEUE_SIZE;
    printf("current_thread = %d\tEND = %d\n", current_thread, THREAD_QUEUE_FINAL_POS);
    printf("Rewriting principal\n");
    makecontext(&principal, nextThread, 0);
    printf("Setting Principal -> %s\n", thread_ready_queue[current_thread]->name);
    setcontext(&(thread_ready_queue[current_thread]->context));
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
  THREAD_QUEUE_FINAL_POS=0;
  current_thread=-1;
  memset(thread_ready_queue, 0, sizeof(dccthread_t *) * THREAD_QUEUE_SIZE);

  getcontext(&manager);
  getcontext(&principal);
  manager.uc_link = NULL;
  setStackProperties(&manager);
  
  
  principal.uc_link = &manager;
  setStackProperties(&principal);
  makecontext(&principal, nextThread, 0);
  dccthread_create("Main", func, param);
  printf("Swapping Manager -> Principal\n");
  setcontext(&principal);
  printf("UE, voltou?\n");
}


dccthread_t * dccthread_create(const char *name, void (*func)(int), int param)
{
  if(is_thread_queue_full())
    return NULL;
  
  thread_ready_queue[THREAD_QUEUE_FINAL_POS] = (dccthread_t*) malloc(sizeof(dccthread_t));
  getcontext(&(thread_ready_queue[THREAD_QUEUE_FINAL_POS]->context));
  thread_ready_queue[THREAD_QUEUE_FINAL_POS]->context.uc_link = &principal;
  setStackProperties(&thread_ready_queue[THREAD_QUEUE_FINAL_POS]->context);
  makecontext(&thread_ready_queue[THREAD_QUEUE_FINAL_POS]->context, (void (*) (void)) func, 1, param);
  thread_ready_queue[THREAD_QUEUE_FINAL_POS]->name = malloc(sizeof(char)*(strlen(name) + 1));
  strcpy(thread_ready_queue[THREAD_QUEUE_FINAL_POS]->name, name);
  
  dccthread_t *ret = thread_ready_queue[THREAD_QUEUE_FINAL_POS];
  printf("Added \"%s\" at %d\n", thread_ready_queue[THREAD_QUEUE_FINAL_POS]->name, THREAD_QUEUE_FINAL_POS);
  THREAD_QUEUE_FINAL_POS++;
  THREAD_QUEUE_FINAL_POS%=THREAD_QUEUE_SIZE;
  
  return ret;
}

void dccthread_yield(void)
{
  thread_ready_queue[THREAD_QUEUE_FINAL_POS] = thread_ready_queue[current_thread];
  thread_ready_queue[current_thread]=NULL;
  int old_index=THREAD_QUEUE_FINAL_POS;
  THREAD_QUEUE_FINAL_POS++;  THREAD_QUEUE_FINAL_POS%=THREAD_QUEUE_SIZE;
  
  printf("Swapping %d -> Principal\n", current_thread);
  swapcontext(&(thread_ready_queue[old_index]->context), &principal);
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
  return thread_ready_queue[current_thread];
}

const char *dccthread_name(dccthread_t *tid)
{
  return tid->name;
}




