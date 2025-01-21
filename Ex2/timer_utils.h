#ifndef _TIMER_UTILS_H_
#define _TIMER_UTILS_H_
#include <sys/time.h>
#include "macros_and_errors.h"

/**********************************************************************
 * UTILS TO DEAL WITH THE TIMER AND THE TIMER'S SIGNALS AT 'UTHREADS' *
 **********************************************************************/

// handling errors delete memory func helper
typedef void (*timer_handler_func)(int);

/**
 * @brief initializing the timer for 'uthreads'.
 *
 * @param quantum - quantum length for the timer
 * @param timer - a pointer to the current 'uthread' timer allocated
 * @param timer_handler - the function to execute when the a quantum expired.
 * @param free_memory_func - a function to delete all allocated memory in a
 * program for a case of an error that requires immediate exit.
 */
void timer_init(int quantum, itimerval *timer, timer_handler_func
timer_handler, free_memory_func delete_all_threads);

/**
 * @brief mask timer signal
 *
 * @param func - a function to delete all allocated memory in a program for
 * a case of 'sigprocmask' error happens in this function.
 */
void mask_timer_signal(free_memory_func func);

/**
 * @brief unmask timer signal
 *
 * @param func - a function to delete all allocated memory in a program for
 * a case of 'sigprocmask' error happens in this function.
 */
void unmask_timer_signal(free_memory_func func);

/**
 * @brief create timer signal set for masking
 *
 * @param func - a function to delete all allocated memory in a program for
 * a case of 'sigemptyset' error happens in this function.
 */
void create_sigset_for_mask(free_memory_func func);

#endif //_TIMER_UTILS_H_
