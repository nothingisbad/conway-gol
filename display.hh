#ifndef CONWAY_DISPLAY_HH
#define CONWAY_DISPLAY_HH

#include "./point_types.hh"

struct Camera : public dTriplet {
  bool AbsoluteP;
  Camera() {
    AbsoluteP = false;
    X = 0;
    Y = 0;
    Z = -1;
  }

  Camera& operator=(const dTriplet& in) {
    *dynamic_cast<dTriplet*>(this) = in;
    return *this;
  }

  Camera& operator+=(const dTriplet& in) {
    X += in.X;
    Y += in.Y;
    Z += in.Z;
    return *this;
  }
};

dTriplet& dTriplet::operator=(Camera& src) {
  return *this = dynamic_cast<dTriplet&>(src);
}

struct screen_data {
  int pixWidth, pixHeight;
  double glWidth, glHeight;
  double aspectRatio;
  constexpr static double dFOV = 0.275; //field of view angle factor

  double dHalfX, dHalfY;
  int iHalfX, iHalfY;

  void resize_window(int width, int height) {
    aspectRatio = (double)width / (double)height;

    /* Protect against a divide by zero */
    if ( height == 0 )
	height = 1;

    /* Setup our viewport. */
    glViewport( 0, 0, ( GLsizei )width, ( GLsizei )height );

    /* change to the projection matrix and set our viewing volume. */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    /* Set our perspective */
    gluPerspective( 45.0f, aspectRatio, 0.1f, 100.0f );

    /* Make sure we're chaning the model view and not the projection */
    glMatrixMode( GL_MODELVIEW );

    /* Reset The View */
    glLoadIdentity( );

    pixWidth = width;
    pixHeight = height;

    glHeight = 3.0;
    glWidth = aspectRatio * glHeight;

    dHalfX = glWidth / 2;
    dHalfY = glHeight / 2;

    iHalfX = static_cast<int>(pixWidth / 2);
    iHalfY = static_cast<int>(pixHeight / 2);
  }

  screen_data(int width = 640, int height = 480) {
    resize_window(width, height);
  }
};

dPair pix_to_gl(const screen_data &screen
		, const iPair &pix
		, const double &depth) {
  double scale_factor = screen.dFOV * depth;
  return dPair( (screen.dHalfX - (((double)pix.X / (double)screen.pixWidth) * screen.glWidth) ) * scale_factor ,
		((((double)pix.Y / (double)screen.pixHeight) * screen.glHeight) - screen.dHalfY) * scale_factor );
}

iPair gl_to_pix(const screen_data &screen, const double &x, const double &y, const double &z) {
  double scale_factor = -(screen.dFOV * z);
  return iPair( (int)(x / (screen.dHalfX * scale_factor)) * screen.iHalfX + screen.iHalfX
		, (int)(y / (screen.dHalfY * scale_factor)) * screen.iHalfY + screen.iHalfY
		);
}


iPair gl_to_pix(const screen_data &screen
		,const dPair& point
		,const double &Z) {
  return gl_to_pix(screen, point.X, point.Y, Z);
}

iPair gl_to_pix(const screen_data &screen
		, const dTriplet &point) {
  return gl_to_pix(screen, point.X, point.Y, point.Z);
}


#endif
