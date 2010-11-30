#ifndef RYAN_CONSOLE_IO_HH
#define RYAN_CONSOLE_IO_HH

#include <iostream>
#include <string>
#include <queue>
#include <boost/thread.hpp>

#include "./point_types.hh"

//todo: must be singleton
class Console {
protected:
  std::queue<std::string> InBuffer, OutBuffer;
  boost::mutex MutexIn, MutexOut;

  void in_push_string_back(const std::string &str) {
    boost::mutex::scoped_lock lock(MutexIn);
    InBuffer.push(str);
  }
    
public:
  void operator()() {
    std::string str;
    while(true) {
      std::cout<<"> "<<std::flush;
      while( std::cin ) {
	std::cin >> str;
	in_push_string_back(str);
      }
      sleep(1);
    }
  }

  std::string in_front() {
    boost::mutex::scoped_lock lock(MutexIn);
    return InBuffer.front();
  }

  void in_pop() {
    boost::mutex::scoped_lock lock(MutexIn);
    InBuffer.pop();
  }

  bool in_empty() {
    boost::mutex::scoped_lock lock(MutexIn);
    return InBuffer.empty() || InBuffer.back().empty();
  }
};


struct parse_consol {
  enum state_type {getting_x = 0, getting_y = 1, getting_z = 2 };
  state_type State;
  
  dTriplet Pos;

  Console &IO;

  parse_consol(Console &io) : IO(io) {
    State = getting_x;
  }

  void operator()(Camera &camera) {
    std::string str;

    while( !IO.in_empty() ) {
      str = IO.in_front();
      IO.in_pop();

      if(str == "print") {
	if(!IO.in_empty()) {
	  str = IO.in_front();
	  IO.in_pop();
	  if(str == "camera") {
	    std::cout<<"Camera at: "<<camera.X<<" "<<camera.Y<<" "<<camera.Z<<std::endl;
	    goto reset_parse_state;
	  } 
	}
	std::cout<<"Last input position: "<<Pos.X<<" "<<Pos.Y<<" "<<Pos.Z<<std::endl;
	goto reset_parse_state;
      } 
      if(str == "absolute") {
	camera.AbsoluteP = true;
	goto reset_parse_state;
      }
      if(str == "relative") {
	camera.AbsoluteP = false;
	goto reset_parse_state;
      }

      switch(State) {
      case getting_x:
	Pos.X = atof( str.c_str() );
	State = getting_y;
      case getting_y:
	if(IO.in_empty()) return;
	str = IO.in_front();
	IO.in_pop();
	Pos.Y = atof(str.c_str() );
	State = getting_z;
      case getting_z:
	if(IO.in_empty()) return;
	str = IO.in_front();
	IO.in_pop();
	Pos.Z = atof(str.c_str() );
	State = getting_x;
	break;
      }

      if(camera.AbsoluteP) {
	camera = Pos;
      } else {
	camera += Pos;
      }
      continue;
    reset_parse_state:
      State = getting_x;
    }
  }
};

#endif
