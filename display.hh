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
  static const double dFOV = 0.275; //field of view angle factor

  double xToEdge, yToEdge;

  void resize_window(int width, int height) {
    /* Height / width ration */
    GLfloat ratio;

    /* Protect against a divide by zero */
    if ( height == 0 )
	height = 1;

    ratio = ( GLfloat )width / ( GLfloat )height;

    /* Setup our viewport. */
    glViewport( 0, 0, ( GLsizei )width, ( GLsizei )height );

    /* change to the projection matrix and set our viewing volume. */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    /* Set our perspective */
    gluPerspective( 45.0f, ratio, 0.1f, 100.0f );

    /* Make sure we're chaning the model view and not the projection */
    glMatrixMode( GL_MODELVIEW );

    /* Reset The View */
    glLoadIdentity( );

    pixWidth = width;
    pixHeight = height;

    aspectRatio = (double)width / (double)height;

    glHeight = 3.0;
    glWidth = aspectRatio * glHeight;

    xToEdge = glWidth / 2;
    yToEdge = glHeight / 2;
  }

  screen_data(int width = 640, int height = 480) {
    resize_window(width, height);
  }

//  dPair pix_to_gl(iPair pix, double depth) {
//    return dPair( (((double)pix.X / (double)pixWidth) * glWidth) - xToEdge ,
//			yToEdge - (((double)pix.Y / (double)pixHeight) * glHeight) 
//			);
//  }
};

dPair pix_to_gl(const screen_data &screen
		, const iPair &pix
		, const double &depth) {

  double scale_factor = screen.dFOV * depth;
  return dPair( ((((double)pix.X / (double)screen.pixWidth) * screen.glWidth) - screen.xToEdge) * scale_factor ,
		(screen.yToEdge - (((double)pix.Y / (double)screen.pixHeight) * screen.glHeight)) * scale_factor );
}


#endif
