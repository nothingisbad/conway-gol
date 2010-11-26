#ifndef POINT_TYPE_HH
#define POINT_TYPE_HH

struct Camera;

struct dPair {
  double X, Y;
  dPair(double x = 0.0, double y = 0.0) {
    X = x;
    Y = y;
  }
};

struct dTriplet : public dPair {
  double Z;

  dTriplet& operator=(Camera& in);
};

struct iPair {
  int X, Y;
};

#endif
