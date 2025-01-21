#ifdef __x86_64__
/* code for 64 bit Intel arch */
typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

#else
/* code for 32 bit Intel arch */
typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

#endif

#ifndef _THREAD_H_
#define _THREAD_H_
#include <csetjmp>
#include "macros_and_errors.h"

#define STACK_SIZE 4096 /* stack size per thread (in bytes) */

typedef void (*thread_entry_point)();
typedef enum {
    READY, BLOCKED, RUNNING
} thread_states;

/**
 * this class represent a single thread
 */
class Thread
{
 private:
    int _id;
    thread_states _state;
    char* _stack;
    int _quantum_num;

 public:
  int quantum_to_wake_me_up = 0;
  sigjmp_buf _env;

  /**
   * Constructor - inits new thread.
   */
  Thread (int id,thread_entry_point entry_point, char* stack);

  /**
   * destructor - delete thread.
   */
  ~Thread ();

  /**
   * increase thread's quantum value by 1
   */
  void increase_quantum();

  /**
   * setter - set new value for _state without any param checks
   *
   * @param state - new state to set
   */
  void set_state (thread_states state);

  /**
   * setter - set new value for _stack without any param checks
   *
   * @param new_stack_pointer - new pointer to stack
   */
  void set_stack_pointer(char* new_stack_pointer);

 /**
  * getter - return the thread's _quantum
  */
  int get_quantum_num () const;

  /**
   * getter - return the thread's _state
   */
  thread_states get_state () const;

  /**
   * getter - return the thread's _id
   */
  int get_ID() const;

  /**
   * getter - return the thread's _stack
   */
  char* get_stack_pointer ();
};


/**
 * @authors - Hebrew University OS course. given as part of the exercise
 * instructions
 *
 * given brief - A translation is required when using an address of a
 * variable. Use this as a black box in your code.
 */
address_t translate_address(address_t addr);

#endif //_THREAD_H_
