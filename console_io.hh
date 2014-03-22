#ifndef RYAN_CONSOLE_IO_HH
#define RYAN_CONSOLE_IO_HH

#include <iostream>
#include <string>
#include <queue>

#include <thread>
#include <mutex>
#include <chrono>

#include <sstream>

#include "./point_types.hh"

using namespace std;

/**
 * Waits for console input and can be
 * queried asynchronously.
 *
 * Should be a singleton.
 */
class Console {
protected:
  std::queue<std::string> InBuffer, OutBuffer;
  std::mutex MutexIn, MutexOut;

  typedef unique_lock<mutex> scoped_lock;

  void in_push_string_back(const std::string &str) {
    scoped_lock(MutexIn);
    InBuffer.push(str);
  }
public:
  void print(string str) {
    scoped_lock(MutexIn);
    scoped_lock(MutexOut);
    cout << str << std::flush;
  }

  /**
   * Thread loop, listens to cin and prints the prompt
   */
  void operator()() {
    std::string str;
    while(true) {
      print("> ");
      while( std::cin >> str )
	in_push_string_back(str);
    }
  }

  std::string in_front() {
    scoped_lock lock(MutexIn);
    return InBuffer.front();
  }

  void in_pop() {
    scoped_lock lock(MutexIn);
    InBuffer.pop();
  }

  bool in_empty() {
    scoped_lock lock(MutexIn);
    return InBuffer.empty() || InBuffer.back().empty();
  }
};

/**
 * Parses a string from the console.
 */
struct ParseConsole {
  enum state_type {getting_x = 0, getting_y = 1, getting_z = 2 };

  /* some of the commands are looking for several strings (a X,Y,Z co-ordinant), if the user put in extra newlines,
     remember which number I'm looking for. */
  state_type State;
  
  dTriplet Pos;

  Console &IO;

  ParseConsole(Console &io) : IO(io) {
    State = getting_x;
  }

  void operator()(Camera &camera) {
    std::string str;
    stringstream out;

    while( !IO.in_empty() ) {
      str = IO.in_front();
      IO.in_pop();

      if(str == "help") {
	out << "Supports a few commands \n"
	    << "  print camera: print the camera position\n"
	    << "  absolute <X> <Y> <Z>: Where X Y Z are floats; set the camera position relitive to the top-left corner of the board.  Z should be negative for the board to be view able. \n"
	    << "  relative <X> <Y> <Z>: Where X Y Z are floats; set the camera position relitive to its current poisition.  Positive Z moves the camera towards the board. \n"
	    << "  quit: exit conway \n"
	    << "> ";
	IO.print( out.str() );
      }

      if(str == "print") {
	if(!IO.in_empty()) {
	  str = IO.in_front();
	  IO.in_pop();
	  if(str == "camera") {
	    out<<"Camera at: "<<camera.X<<" "<<camera.Y<<" "<<camera.Z<< "\n> ";
	    IO.print( out.str() );
	    goto reset_parse_state;
	  } 
	}
	out << "Last input position: "<<Pos.X<<" "<<Pos.Y<<" "<<Pos.Z << "\n> ";
	IO.print( out.str() );
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

      if(str == "quit") exit(0);

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
      IO.print("> ");
      continue;

    reset_parse_state:
      State = getting_x;
    }
  }
};

#endif
