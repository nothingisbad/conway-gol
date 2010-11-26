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
  int State;
  dTriplet Pos;

  Console &IO;

  parse_consol(Console &io) : IO(io) {
    State = 0;
  }

  void operator()(Camera &camera) {
    std::string str;

    while( !IO.in_empty() ) {
      str = IO.in_front();
      IO.in_pop();

      if(str == "print") {
	std::cout<<Pos.X<<" "<<Pos.Y<<" "<<Pos.Z<<std::endl;
	continue;
      } if(str == "absolute") {
	camera.AbsoluteP = true;
	continue;
      } else if(str == "relative") {
	camera.AbsoluteP = false;
	continue;
      } 

      switch(State) {
      case 0:
	Pos.X = atof( str.c_str() );
	std::cout<<"Have X: "<<Pos.X<<" "<<std::flush;
	++State;
      case 1:
	if(IO.in_empty()) return;
	str = IO.in_front();
	IO.in_pop();
	Pos.Y = atof(str.c_str() );
	std::cout<<"Have y: "<<Pos.Y<<" "<<std::flush;
	++State;
      case 2:
	if(IO.in_empty()) return;
	str = IO.in_front();
	IO.in_pop();
	Pos.Z = atof(str.c_str() );
	std::cout<<"Have z: "<<Pos.Z<<std::endl;
	++State;
      case 3:
	State = 0;
	break;
      }

      if(camera.AbsoluteP) {
	camera = Pos;
      } else {
	camera += Pos;
      }
    }
  }
};

#endif
