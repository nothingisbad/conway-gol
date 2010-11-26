CPP = g++ -Wall 

conway: ./conway.cc ./game_logic.hh ./display.hh ./console_io.hh
	$(CPP) ./conway.cc -o conway -lGL -lGLU `sdl-config --cflags --libs` -lboost_thread

console_io: ./test_console_io.cc ./console_io.hh
	$(CPP) test_console_io.cc -o console_io -lboost_thread

clean:
	@if [ -e ./conway ]; then rm ./conway; fi
	@if [ -e ./console_io ]; then rm ./console_io; fi
