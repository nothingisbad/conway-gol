#include "./console_io.hh"
#include <unistd.h>

int main() {
  Console io;
  
  boost::thread thrd( boost::ref(io) );

  while(true) {
    sleep(1);

    if(!io.in_empty()) {
      std::cout<<io.in_top()<<std::endl;
      io.in_pop();
    } else
      std::cout<<"buffer empty."<<std::endl;
  }
  return 0;
}
