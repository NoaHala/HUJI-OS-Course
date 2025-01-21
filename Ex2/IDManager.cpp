#include "IDManager.h"

// Constructor - see full documentation in header file
IDManager::IDManager (){
  for (int id = 0; id < MAX_THREAD_NUM; id++)
  {
    _ID_heap.push (id);
  }
}

// return min id available - see full documentation in header file
int IDManager::get_min_id_available ()
{
  if(this->_ID_heap.empty()){
    return -1;
  }

  int min_id = this->_ID_heap.top();
  this->_ID_heap.pop();
  return min_id;
}

// return unused id to heap - see full documentation in header file
void IDManager::return_unused_id (int id)
{
  this->_ID_heap.push (id);
}


