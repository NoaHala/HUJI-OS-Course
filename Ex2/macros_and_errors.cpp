#include "macros_and_errors.h"

//error handling functions - see full documentation in header file
void handle_error(error_type type,
                  const std::string& msg,
                  free_memory_func free_func){
  //checks error type and prints accordingly
  if (type == SYSTEM){
    std::cerr << SYSTEM_ERROR_MSG << msg << "\n" << std::endl;
    free_func();
    exit (ERROR_EXIT_CODE);
  }
  else{
    std::cerr << LIBRARY_ERROR_MSG << msg << "\n" << std::endl;
  }
}