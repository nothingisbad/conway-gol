#ifndef CONWAY_GAME_LOGIC_HH
#define CONWAY_GAME_LOGIC_HH

#include <string>


struct game_grid {
  int Height, Width;
  bool *Current, *Buffer;

  game_grid() {
    Height = 100;
    Width = 100;
    Current = new bool[Height * Width];
    Buffer = new bool[Height * Width];
    for(int i = 0; i < Height - 1; ++i) {
      for(int j = 0; j < Width; ++j) {
	Current[(Width * i) + j] = false;
	Buffer[(Width * i) + j] = false;
      }
    }
  }

  ~game_grid() {
    delete[] Current;
    delete[] Buffer;
  }

  int height() {
    return Height;
  }

  int width() {
    return Width;
  }

  bool operator()(int x, int y) {
    return Current[(Width * x) + y];
  }

  void buffer_set(int x, int y, bool value) {
    Buffer[(Width * x) + y] = value;
  }

  void click(int x, int y) {
    Current[(Width * x) + y] ^= true;
  }
};

inline void iterate(game_grid &grid) {
  int sum = 0;
  bool *tmp;

  for(int i = 1; i < grid.Height - 1; ++i) {
    for(int j = 1; j < grid.Width - 1; ++j) {
      sum = grid(i, j + 1) + grid(i, j - 1) + grid(i + 1, j) + grid(i - 1,j) + grid(i + 1,j + 1) + grid(i + 1,j - 1) + grid(i - 1,j + 1) + grid(i - 1,j - 1);
      if( (sum == 3)
	  || (grid(i, j) 
	      && (sum == 2)) ) {
	grid.buffer_set(i, j, true);
      } else {
	grid.buffer_set(i,j, false);
      }
      sum = 0;
    }
  }

  tmp = grid.Current;
  grid.Current = grid.Buffer;
  grid.Buffer = tmp;
}


#endif
