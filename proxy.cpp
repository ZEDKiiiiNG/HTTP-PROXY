#include "HTTPProxy.hpp"
#include "assert.h"

int main() {
  try{
    HTTPProxy httpproxy("12345");
    httpproxy.startRun();
    
  }catch(myException &e){
    std::cout<< e.what() << std::endl;
  }
  return EXIT_SUCCESS;
}
