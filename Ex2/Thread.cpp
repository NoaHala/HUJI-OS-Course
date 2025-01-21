#include "Thread.h"
#include <csignal>

#ifdef __x86_64__
/* code for 64 bit Intel arch */

// blackbox function - see full documentation in header file
address_t translate_address(address_t addr)
{
  address_t ret;
  asm volatile("xor    %%fs:0x30,%0\n"
               "rol    $0x11,%0\n"
      : "=g" (ret)
      : "0" (addr));
  return ret;
}

#else
/* code for 32 bit Intel arch */

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#endif

// Constructor - see full documentation in header file
Thread::Thread (int id, thread_entry_point entry_point, char* stack)
{
  _id = id;
  _state = READY;
  _quantum_num = 0;
  _stack = stack;
//  try
//  {
//    _stack = new char[STACK_SIZE];
//  }
//  catch (const std::bad_alloc& error)
//  {
//    handle_error (SYSTEM, ALLOCATION_ERROR, nullptr);
//  }

  address_t sp = (address_t) _stack + STACK_SIZE - sizeof(address_t);
  address_t pc = (address_t) entry_point;
  sigsetjmp(_env, 1);
  (_env->__jmpbuf)[JB_SP] = translate_address(sp);
  (_env->__jmpbuf)[JB_PC] = translate_address(pc);
  sigemptyset(&_env->__saved_mask);
}

// destructor - see full documentation in header file
Thread::~Thread ()
{
  if (_stack != nullptr){
    delete _stack;
    _stack = nullptr;
  }
}

// setter - see full documentation in header file
void Thread::set_state (thread_states state)
{
  _state = state;
}

// getter - see full documentation in header file
thread_states Thread::get_state() const{
  return _state;
}

// getter - see full documentation in header file
int Thread:: get_ID() const{
  return _id;
}

// getter - see full documentation in header file
int Thread::get_quantum_num () const
{
  return _quantum_num;
}

// increase quantum by 1 - see full documentation in header file
void Thread::increase_quantum(){
  _quantum_num++;
}