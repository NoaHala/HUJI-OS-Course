#ifndef _MACROS_AND_ERRORS_H_
#define _MACROS_AND_ERRORS_H_

#include <string>
#include <iostream>


/**********************
 *   GENERAL MACROS   *
 **********************/
#define SECOND 1000000
#define MAIN_THREAD_ID 0

/************************
 *   EXIT CODE MACROS   *
 ************************/
#define ERROR_EXIT_CODE 1
#define FAILURE (-1)
#define SUCCESS 0

/****************************
 *   ERROR MESSAGE MACROS   *
 ****************************/
#define SYSTEM_ERROR_MSG "system error: "
#define LIBRARY_ERROR_MSG "library error: "
#define NON_POSITIVE_QUANTUM_ERROR "quantum for the threads must be positive."
#define SIGPROCMASK_ERROR "calling sigprocmask() failed"
#define SIGEMPTYSET_ERROR "calling sigemptyset() failed"
#define SIGADDSET_ERROR "calling sigaddset() failed"
#define REACHED_MAX_THREADS_ERROR "reached max threads num possible"
#define THREAD_ENTRY_POINT_NULL "can't create thread with null entry point"
#define DELETE_NON_EXIST_ID_ERROR "can't delete thread with non-exist id"
#define ALLOCATION_ERROR "allocation failed"
#define SIGACTION_ERROR "calling sigaction() failed"
#define SETITTIMER_ERROR "calling setittimer() failed"
#define BLOCK_NON_EXIST_THREAD_ERROR "can't block non-exist thread"
#define BLOCK_MAIN_THREAD_ERROR "can't block the main thread"
#define RESUME_NON_EXIST_THREAD_ERROR "can't resume non-exist thread"
#define SLEEP_MAIN_THREAD_ERROR "can't make the main thread sleep"
#define QUANTUM_NOT_EXIST_ERROR "can't get quantum of non-exist thread"

/******************************
 *   HANDLING ERROR HELPERS   *
 ******************************/
typedef void (*free_memory_func)();
typedef enum {
    SYSTEM, LIBRARY
} error_type;

/**
 * @brief handling error according to different parameters
 * @param type - type of the error SYSTEM/LIBRARY
 * @param msg - the specific error msg to print
 * @param free_func - if error requires exit the code, run this function
 * to delete all allocated memory before
 */
void handle_error(error_type type,
                  const std::string& msg,
                  free_memory_func free_func);

#endif //_MACROS_AND_ERRORS_H_
