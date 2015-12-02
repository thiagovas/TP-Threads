/* By Thiago Silva */

#include "dccthread.h"


/* Variable that keeps the index of the thread running currently. */
int current_thread;

/* Variables that keep the final position of the queue */
int THREAD_QUEUE_FINAL_POS;

/* Queue that keeps the threads ready to execute */
dccthread_t *thread_ready_queue[THREAD_QUEUE_SIZE];

/* The two main threads of the program, initialized at the init function */
ucontext_t manager, exiter;

/* Variable used to create new thread id's */
int neueId;


/* TIMER VARIABLES */
  struct sigevent timer_event;
  struct sigaction timerAction, oldTimerAction;
  struct sigaction oldTimerAction;
  struct itimerspec deltaTime;
  timer_t timer;
/* END - TIMER VARIABLES */



/******************* NOT LISTED *******************/

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

/* Method that blocks interrupts */
void blockInterrupts()
{
  sigset_t signal_set;
  sigaddset(&signal_set, SIGRTMIN);
  sigprocmask(SIG_BLOCK, &signal_set, NULL);
}

/* This method initialized the timer */
void init_timer()
{
  timer_event.sigev_notify = SIGEV_SIGNAL;
  timer_event.sigev_signo = SIGRTMIN;
  timer_create(CLOCK_PROCESS_CPUTIME_ID, &timer_event, &timer);

  timerAction.sa_handler = (void (*)())dccthread_yield;
  timerAction.sa_flags  = SA_SIGINFO;
  sigaction(SIGRTMIN, &timerAction, NULL);

  deltaTime.it_interval.tv_sec = deltaTime.it_value.tv_sec = 0;
  deltaTime.it_value.tv_nsec = deltaTime.it_interval.tv_nsec = 10LL;
  timer_settime(&timer, 0, &deltaTime, NULL);
}


/* Unblocks the interrupts */
void unblockInterrupts()
{
  sigset_t signal_set;
  sigaddset(&signal_set, SIGRTMIN);
  timer_delete(timer);
  init_timer();
  sigprocmask(SIG_UNBLOCK, &signal_set, NULL);
}


/* This function send the current thread to the end of the queue. */
int send_to_the_end()
{
  thread_ready_queue[THREAD_QUEUE_FINAL_POS] = thread_ready_queue[current_thread];
  thread_ready_queue[current_thread]=NULL;
  int old_index=THREAD_QUEUE_FINAL_POS;
  THREAD_QUEUE_FINAL_POS++;  THREAD_QUEUE_FINAL_POS%=THREAD_QUEUE_SIZE;
	
	return old_index;
}



/* This function releases every thread wating for the [index] thread */
void free_waitings(int index)
{
	int i=index;
	while(i != THREAD_QUEUE_FINAL_POS)
	{
		if(thread_ready_queue[i]->waiting_for == thread_ready_queue[index]->id)
			thread_ready_queue[i]->waiting_for = -1;
		i=(i+1)%THREAD_QUEUE_SIZE;
	}
}

void nextThread()
{
    blockInterrupts();
    if(is_thread_queue_empty()) return;
		printf("Running: nextThread\n");
    
		while(1)
		{
			current_thread++;
			current_thread%=THREAD_QUEUE_SIZE;
      if(is_thread_queue_empty()) return;
      printf("%s waiting for %d\n", thread_ready_queue[current_thread]->name, thread_ready_queue[current_thread]->waiting_for);
			if(thread_ready_queue[current_thread]->waiting_for != -1)
				send_to_the_end();
			else break;
		}
    getcontext(&manager);
    makecontext(&manager, nextThread, 0);
    
    unblockInterrupts();
    printf("Running \"%s\"\n", thread_ready_queue[current_thread]->name);
    setcontext(&(thread_ready_queue[current_thread]->context));
}

/* Method that initializes the stack of a context */
void setStackProperties(ucontext_t *context)
{
  context->uc_stack.ss_sp = malloc(SIGSTKSZ);
  context->uc_stack.ss_size = SIGSTKSZ;
  context->uc_stack.ss_flags = 0;
}


/* This method frees every chunk of memory allocated for the [index] thread. */
void free_dcc_thread(int index)
{
	free(thread_ready_queue[index]);
  thread_ready_queue[index]=NULL;
}

/* This function checks if the [id] thread is done running. */
int is_thread_done(int id)
{
  int i=current_thread;
  while(i != THREAD_QUEUE_FINAL_POS)
  {
    if(thread_ready_queue[i]->id == id) return 0;
    i=(i+1)%THREAD_QUEUE_SIZE;
  }
  return 1;
}


dccthread_t* non_blocking_thread_create(const char *name, void (*func)(int), int param)
{
  if(is_thread_queue_full())
    return NULL;
  
  if(thread_ready_queue[THREAD_QUEUE_FINAL_POS] != NULL)
    free_dcc_thread(THREAD_QUEUE_FINAL_POS);
  thread_ready_queue[THREAD_QUEUE_FINAL_POS] = (dccthread_t *) malloc(sizeof(dccthread_t *));
  getcontext(&(thread_ready_queue[THREAD_QUEUE_FINAL_POS]->context));
  thread_ready_queue[THREAD_QUEUE_FINAL_POS]->context.uc_link = &manager;
  setStackProperties(&(thread_ready_queue[THREAD_QUEUE_FINAL_POS]->context));
  makecontext(&(thread_ready_queue[THREAD_QUEUE_FINAL_POS]->context), (void (*) (void)) func, 1, param);
  thread_ready_queue[THREAD_QUEUE_FINAL_POS]->name = name;
  thread_ready_queue[THREAD_QUEUE_FINAL_POS]->waiting_for=-1;
  thread_ready_queue[THREAD_QUEUE_FINAL_POS]->id = neueId;
  
  neueId++;
  if(neueId < 0) neueId=0;

  dccthread_t *ret = thread_ready_queue[THREAD_QUEUE_FINAL_POS];
  THREAD_QUEUE_FINAL_POS++;
  THREAD_QUEUE_FINAL_POS%=THREAD_QUEUE_SIZE;
  
  return ret;
}


/******************* END - NOT LISTED *******************/






void dccthread_init(void (*func)(int), int param)
{
  init_timer();
  THREAD_QUEUE_FINAL_POS=neueId=0;
  current_thread=-1;
  memset(thread_ready_queue, 0, sizeof(dccthread_t *) * THREAD_QUEUE_SIZE);
  
  getcontext(&manager);
  getcontext(&exiter);

  manager.uc_link = &exiter;
  setStackProperties(&manager);
  setStackProperties(&exiter);
  
  makecontext(&manager, nextThread, 0);
  non_blocking_thread_create("Main", func, param);
  
  swapcontext(&exiter, &manager);
  exit(EXIT_SUCCESS);
}


dccthread_t * dccthread_create(const char *name, void (*func)(int), int param)
{
  blockInterrupts();
  dccthread_t *ret = non_blocking_thread_create(name, func, param);
  unblockInterrupts();
  return ret;
}


void dccthread_yield(void)
{
  blockInterrupts();
  int old_index = send_to_the_end();
  printf("Swapping %d -> Manager\n", thread_ready_queue[old_index]->id);
  swapcontext(&(thread_ready_queue[old_index]->context), &manager);
}

void dccthread_exit(void)
{
  blockInterrupts();
  printf("%d exited\n", thread_ready_queue[current_thread]->id);
	free_waitings(current_thread);
  setcontext(&manager);
} 

void dccthread_wait(dccthread_t *tid)
{
  blockInterrupts();
	if(tid==NULL)
	{
		printf("NULL pointer on dccthread_wait function\n");
		exit(EXIT_FAILURE);
	}
  
  printf("Asking %d to wait for %d\n", thread_ready_queue[current_thread]->id, tid->id);
	if(is_thread_done(tid->id))
    return;
  int index = send_to_the_end();
  thread_ready_queue[index]->waiting_for = tid->id;
	swapcontext(&(thread_ready_queue[index]->context), &manager);
  unblockInterrupts();
}

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


