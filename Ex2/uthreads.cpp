//#ifndef _UTHREADS_CPP
//#define _UTHREADS_CPP
#include <unordered_map>
#include <queue>
#include <unordered_set>
#include "uthreads.h"
#include "timer_utils.h"
#include "Thread.h"
#include "IDManager.h"
#include "macros_and_errors.h"

/*************************
 *   EARLY DECLARATIONS  *
 *************************/
void timer_handler(int sig);
void remove_from_ready_queue (int tid);
void wake_up_thread (Thread *thread);

/**********************
 *   DATA STRUCTURES  *
 **********************/
std::unordered_map<int, Thread*> cur_threads;
std::queue<Thread*> ready_threads;
std::unordered_set<int> all_cur_threads_ids;
std::unordered_set<Thread*> sleeping_threads;

/************************
 *   GLOBAL VARIABLES   *
 ************************/
struct itimerval timer = *new itimerval;
IDManager id_manager = *new IDManager;
Thread* cur_running_thread;
int quantum;
int all_quantums = 1;

/**************************
 *   SAFE DELETE METHODS  *
 **************************/
//safe delete single thread
void safe_delete_thread(int tid){
  delete cur_threads[tid];
  cur_threads[tid] = nullptr;
  cur_threads.erase (tid);
}

//delete all threads
void delete_all_threads(){
  for (std::pair<const int, Thread *> thread : cur_threads){
    safe_delete_thread (thread.first);
  }
}

/**************************
 *  OTHER HELPER METHODS  *
 **************************/

/*
 * wake up sleeping thread
 */
void wake_up_thread (Thread *thread)
{
  thread->set_state (READY);
  ready_threads.push (thread);
  thread->quantum_to_wake_me_up = 0;
}

/*
 * iterate over the sleeping threads and checks if they suppose to wake up
 */
void check_for_threads_to_wake_up(){
  //wake up sleeping threads
  for (auto thread_iterator = sleeping_threads.begin();
       thread_iterator != sleeping_threads.end();) {
    Thread* thread = *thread_iterator;
    if (thread->quantum_to_wake_me_up == all_quantums) {
      wake_up_thread(thread);
      thread_iterator = sleeping_threads.erase(thread_iterator);
    } else {
      thread_iterator++;
    }
  }
}

/*
 * runs the next thread in the ready queue
 */
void run_next_thread(){
  Thread* next_thread = ready_threads.front();
  next_thread->set_state (RUNNING);
  cur_running_thread = next_thread;
  ready_threads.pop();

  //increase quantum's counters
  next_thread->increase_quantum();
  all_quantums++;

  check_for_threads_to_wake_up();

  unmask_timer_signal (delete_all_threads);
  siglongjmp(next_thread->_env, 1);
}

/*
 * executes every time a quantum ends and the timer expired
 */
void timer_handler(int sig){
  mask_timer_signal (delete_all_threads);

  int ret_val = sigsetjmp(cur_running_thread->_env, 1);
  bool did_just_save_bookmark = ret_val == 0;
  if (did_just_save_bookmark)
  {
    ready_threads.push (cur_running_thread);
    cur_running_thread->set_state (READY);
    run_next_thread();
  }
}

/*
 * remove a thread with id = tid from the ready queue
 */
void remove_from_ready_queue (int tid)
{
  int queue_size = (int)ready_threads.size();

  for (int i = 0; i < queue_size; i++)
  {
    Thread* temp = ready_threads.front();
    ready_threads.pop();
    if (temp->get_ID() != tid){
      ready_threads.push (temp);
    }
  }
}

/***********************************
 *  PUBLIC METHODS IMPLEMENTATION  *
 ***********************************/

/**
 * @brief initializes the thread library.
 *
 * Full documentation in "uthreads.h".
*/
int uthread_init(int quantum_usecs){

  // checks for valid quantum
  if(quantum_usecs <= 0){
    handle_error (LIBRARY,NON_POSITIVE_QUANTUM_ERROR, nullptr);
    return FAILURE;
  }

  //set quantum
  quantum = quantum_usecs;

  //mask signals
  create_sigset_for_mask(delete_all_threads);
  mask_timer_signal(delete_all_threads);

  //create timer
  timer_init (quantum, &timer, timer_handler, delete_all_threads);

  //create main thread (id=0)
  int cur_thread_id = id_manager.get_min_id_available();

  char* new_stack;
  try
  {
    new_stack = new char[STACK_SIZE];
  }
  catch (const std::bad_alloc& error)
  {
    handle_error (SYSTEM, ALLOCATION_ERROR, nullptr);
  }

  Thread *main_thread;
  try
  {
    main_thread = new Thread (cur_thread_id, nullptr,new_stack);
  }
  catch (const std::bad_alloc& error)
  {
    delete new_stack;
    handle_error (SYSTEM, ALLOCATION_ERROR, delete_all_threads);
  }
  main_thread->increase_quantum();

  //add all relevant DS
  cur_threads[cur_thread_id] = main_thread;
  main_thread->set_state (RUNNING);
  cur_running_thread = main_thread;
  all_cur_threads_ids.insert (cur_thread_id);

  unmask_timer_signal(delete_all_threads);
  return SUCCESS;
}

/**
 * @brief Creates a new thread, whose entry point is the function entry_point with the signature
 * void entry_point(void).
 *
 *Full documentation in "uthreads.h".
*/
int uthread_spawn(thread_entry_point entry_point){
  mask_timer_signal(delete_all_threads);

  //deal with errors and fails
  if(entry_point == nullptr){
    handle_error (LIBRARY, THREAD_ENTRY_POINT_NULL, nullptr);
    unmask_timer_signal(delete_all_threads);
    return FAILURE;
  }

  int new_id = id_manager.get_min_id_available();
  if(new_id == -1){
    handle_error (LIBRARY, REACHED_MAX_THREADS_ERROR, nullptr);
    unmask_timer_signal(delete_all_threads);
    return FAILURE;
  }

  //create new thread
  char* new_stack;
  try
  {
    new_stack = new char[STACK_SIZE];
  }
  catch (const std::bad_alloc& error)
  {
    handle_error (SYSTEM, ALLOCATION_ERROR, nullptr);
  }

  Thread *new_thread;
  try
  {
    new_thread = new Thread (new_id, entry_point, new_stack);
  }
  catch (const std::bad_alloc& error)
  {
    delete new_stack;
    handle_error (SYSTEM, ALLOCATION_ERROR, delete_all_threads);
  };

  new_thread->set_state (READY);

  //add to all relevant DS
  cur_threads[new_id] = new_thread;
  all_cur_threads_ids.insert (new_id);
  ready_threads.push (new_thread);

  unmask_timer_signal(delete_all_threads);
  return new_id;
}

/**
 * @brief Terminates the thread with ID tid and deletes it from all relevant control structures.
 *
 * Full documentation in "uthreads.h".
*/
int uthread_terminate(int tid){
  mask_timer_signal(delete_all_threads);

  //checks for terminate zero
  if(tid == MAIN_THREAD_ID){
    delete_all_threads();
    unmask_timer_signal(nullptr);
    exit(EXIT_SUCCESS);
  }

  //check for non-exist tid
  if(all_cur_threads_ids.find (tid) == all_cur_threads_ids.end()){
    handle_error (LIBRARY, DELETE_NON_EXIST_ID_ERROR, nullptr);
    unmask_timer_signal(delete_all_threads);
    return FAILURE;
  }

  //checks if thread tries to terminate itself
  bool terminate_myself = cur_running_thread->get_ID() == tid;

  //free memory and remove from thread structures
  remove_from_ready_queue(tid);
  sleeping_threads.erase (cur_threads[tid]);
  all_cur_threads_ids.erase (tid);
  safe_delete_thread(tid);
  id_manager.return_unused_id (tid);

  if (terminate_myself){
    timer_init (quantum, &timer, timer_handler, delete_all_threads);
    run_next_thread();
  }

  unmask_timer_signal(delete_all_threads);
  return SUCCESS;
}

/**
 * @brief Blocks the thread with ID tid. The thread may be resumed later using uthread_resume.
 *
 * Full documentation in "uthreads.h".
*/
int uthread_block(int tid){
  mask_timer_signal (delete_all_threads);

  //checks if id exists
  if (all_cur_threads_ids.find (tid) == all_cur_threads_ids.end()){
    handle_error (LIBRARY,BLOCK_NON_EXIST_THREAD_ERROR, nullptr);
    unmask_timer_signal (delete_all_threads);
    return FAILURE;
  }

  //checks if it's the main thread
  if (tid == MAIN_THREAD_ID){
    handle_error (LIBRARY,BLOCK_MAIN_THREAD_ERROR, nullptr);
    unmask_timer_signal (delete_all_threads);
    return FAILURE;
  }

  //change thread's state to block
  cur_threads[tid]->set_state (BLOCKED);

  //check if thread trys to block itself
  if(cur_running_thread->get_ID() == tid){
    int ret_val = sigsetjmp(cur_running_thread->_env, 1);
    bool did_just_save_bookmark = ret_val == 0;
    if (did_just_save_bookmark)
    {
      timer_init (quantum, &timer, timer_handler, delete_all_threads);
      run_next_thread();
    }
  }
  else{
    remove_from_ready_queue (tid);
  }

  unmask_timer_signal (delete_all_threads);
  return SUCCESS;
}

/**
 * @brief Resumes a blocked thread with ID tid and moves it to the READY state.
 *
 * Full documentation in "uthreads.h".
*/
int uthread_resume(int tid){
  mask_timer_signal (delete_all_threads);

  //checks if id exists
  if (all_cur_threads_ids.find (tid) == all_cur_threads_ids.end()){
    handle_error (LIBRARY,RESUME_NON_EXIST_THREAD_ERROR, nullptr);
    unmask_timer_signal (delete_all_threads);
    return FAILURE;
  }

  //checks if it's in BLOCK state
  if (cur_threads[tid]->get_state() == BLOCKED &&
  cur_running_thread->quantum_to_wake_me_up == 0){
    cur_threads[tid]->set_state (READY);
    ready_threads.push (cur_threads[tid]);
  }

  unmask_timer_signal (delete_all_threads);
  return SUCCESS;
}


/**
 * @brief Blocks the RUNNING thread for num_quantums quantums.
 *
 * Immediately after the RUNNING thread transitions to the BLOCKED state a scheduling decision should be made.
 * After the sleeping time is over, the thread should go back to the end of the READY queue.
 * If the thread which was just RUNNING should also be added to the READY queue, or if multiple threads wake up
 * at the same time, the order in which they're added to the end of the READY queue doesn't matter.
 * The number of quantums refers to the number of times a new quantum starts, regardless of the reason. Specifically,
 * the quantum of the thread which has made the call to uthread_sleep isn’t counted.
 * It is considered an error if the main thread (tid == 0) calls this function.
 *
 * @return On success, return 0. On failure, return -1.
*/
int uthread_sleep(int num_quantums){
  mask_timer_signal (delete_all_threads);

  if (cur_running_thread == MAIN_THREAD_ID){
    handle_error (LIBRARY, SLEEP_MAIN_THREAD_ERROR, nullptr);
    unmask_timer_signal (delete_all_threads);
    return FAILURE;
  }

  //saves the cur_thread state
  int ret_val = sigsetjmp(cur_running_thread->_env, 1);
  bool did_just_save_bookmark = ret_val == 0;
  if (did_just_save_bookmark)
  {
    cur_running_thread->set_state (BLOCKED);
    cur_running_thread->quantum_to_wake_me_up = all_quantums + num_quantums + 1;
    sleeping_threads.insert (cur_running_thread);

    timer_init (quantum, &timer, timer_handler, delete_all_threads);
    run_next_thread();
  }

  unmask_timer_signal (delete_all_threads);
  return SUCCESS;
}

/**
 * @brief Returns the thread ID of the calling thread.
 *
 * Full documentation in "uthreads.h".
*/
int uthread_get_tid(){
  return cur_running_thread->get_ID();
}

/**
 * @brief Returns the total number of quantums since the library was initialized, including the current quantum.
 *
 * Full documentation in "uthreads.h".
*/
int uthread_get_total_quantums(){
  return all_quantums;
}

/**
 * @brief Returns the number of quantums the thread with ID tid was in RUNNING state.
 *
 * Full documentation in "uthreads.h".
*/
int uthread_get_quantums(int tid){
  mask_timer_signal (delete_all_threads);

  //checks if id exists
  if (all_cur_threads_ids.find (tid) == all_cur_threads_ids.end()){
    handle_error (LIBRARY,QUANTUM_NOT_EXIST_ERROR, nullptr);
    unmask_timer_signal (delete_all_threads);
    return FAILURE;
  }

  unmask_timer_signal (delete_all_threads);
  return cur_threads[tid]->get_quantum_num();
}

//#endif