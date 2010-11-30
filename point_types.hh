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

  dTriplet() {
    X = Y = Z = 0.0;
  }

  dTriplet(double x, double y, double z) : dPair(x, y), Z(z) {}

  dTriplet(const dPair &pair, const double &z = 0.0) {
    X = pair.X;
    Y = pair.Y;
    Z = z;
  }

  dTriplet& operator=(Camera& in);
  dTriplet& operator=(const dPair &in) {
    X = in.X;
    Y = in.Y;
    return *this;
  }
  dTriplet& operator-() {
    X = -X;
    Y = -Y;
    Z = -Z;
    return *this;
  }

  const dTriplet operator+(const dTriplet& input) {
    return dTriplet(input.X + X, input.Y + Y, input.Z + Z);
  }

  const dTriplet operator-(const dTriplet& input) {
    return dTriplet(X - input.X, Y - input.Y, Z - input.Z);
  }
};

struct iPair {
  int X, Y;
  iPair(int x = 0, int y = 0) : X(x), Y(y) {}

  iPair& operator-() {
    X = -X;
    Y = -Y;
    return *this;
  }
};

#endif
