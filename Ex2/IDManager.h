#include <stdlib.h>
#include <queue>

#ifndef _IDMANAGER_H_
#define _IDMANAGER_H_

#define MAX_THREAD_NUM 100 /* maximal number of threads */

/**
 * this class handles the id assignment to the threads.
 */
class IDManager{

 private:
  // heap containing all the available id numbers
  std::priority_queue<int, std::vector<int>, std::greater<int>> _ID_heap;

 public:

  /**
   * Constructor.
   * init the ID manager by filling the new heap in ids from 0 to 99.
   */
  IDManager ();
  
  /**
   * Finds the smallest id available, deletes it from the heap and returns it.
   *
   * @return if the heap empty, returns -1, else returns the smallest id
   * number available.
   */
  int get_min_id_available();
  
  /**
   * gets a previously assigned id for a terminated thread and returns it to
   * the available ids heap so that it can be reused.
   * @note - For efficiency reasons, the given ID is not checked before being
   * returned to the stack.
   *
   * @param id - An ID number that has become available again.
   */
  void return_unused_id(int id);
};



#endif //_IDMANAGER_H_
