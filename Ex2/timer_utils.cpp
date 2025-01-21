#include <cstdio>
#include <csignal>
#include "timer_utils.h"

sigset_t cur_sigset;
void sig_handler(timer_handler_func timer_handler,
                 free_memory_func delete_all_threads);

// mask timer signal - see full documentation in header file
void mask_timer_signal(free_memory_func delete_all_threads){
  if (sigprocmask (SIG_SETMASK, &cur_sigset, nullptr) == FAILURE){
    handle_error (SYSTEM, SIGPROCMASK_ERROR, delete_all_threads);
  }
}

// unmask timer signal - see full documentation in header file
void unmask_timer_signal(free_memory_func delete_all_threads){
  if (sigprocmask (SIG_UNBLOCK, &cur_sigset, nullptr) == FAILURE){
    handle_error (SYSTEM, SIGPROCMASK_ERROR, delete_all_threads);
  }
}

// creates set of timer signal - see full documentation in header file
void create_sigset_for_mask(free_memory_func delete_all_threads){
  if (sigemptyset (&cur_sigset) == FAILURE){
    handle_error (SYSTEM, SIGEMPTYSET_ERROR,delete_all_threads);
  }
  if (sigaddset (&cur_sigset,SIGVTALRM) == FAILURE){
    handle_error (SYSTEM, SIGADDSET_ERROR, delete_all_threads);
  }
}

// init timer for a new 'uthread' - see full documentation in header file
void timer_init(int quantum, itimerval *timer, timer_handler_func
timer_handler, free_memory_func delete_all_threads){

  //set the signal handler for SIGVTALRM
  sig_handler(timer_handler, delete_all_threads);

  // set time intervals, seconds + microseconds
  timer->it_value.tv_sec = quantum/SECOND;
  timer->it_value.tv_usec = quantum%SECOND;
  timer->it_interval.tv_sec = quantum/SECOND;
  timer->it_interval.tv_usec = quantum%SECOND;

  // Start the virtual timer. checks for error.
  if (setitimer(ITIMER_VIRTUAL, timer, nullptr))
  {
    handle_error (SYSTEM, SETITTIMER_ERROR, delete_all_threads);
  }
}

// helper - set the timer helper as the function to execute when quantum ends
void sig_handler(timer_handler_func timer_handler,
                 free_memory_func delete_all_threads){
  struct sigaction sa = {nullptr};

  // Install timer_handler as the signal handler for SIGVTALRM.
  sa.sa_handler = timer_handler;
  if (sigaction(SIGVTALRM, &sa, nullptr) < 0)
  {
    handle_error (SYSTEM, SIGACTION_ERROR, delete_all_threads);
  }
}