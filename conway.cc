#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"

#include <iostream>

#include "./game_logic.hh"
#include "./display.hh"
#include "./console_io.hh"

/* screen width, height, and bit depth */
#define SCREEN_BPP     16

struct dumb_clock {
  unsigned int Ticks;
  dumb_clock() {
    Ticks = 0;
  }

  void inc() {
    ++Ticks;
  }
};

struct dumb_timer {
  unsigned int Began, DeltaT, Ending;
  dumb_clock &Clock;

  dumb_timer(dumb_clock &clock) : Clock(clock) {
  }

  void reset() {
    Began = Clock.Ticks;
    Ending = Began + DeltaT;
  }

  void how_long(unsigned int tick) {
    DeltaT = tick;
  }

  bool done() {
    return Clock.Ticks >= Ending;
  }
};

/* This is our SDL surface */
SDL_Surface *surface;

/* function to release/destroy our resources and restoring the old desktop */
void Quit( int returnCode ) {
    /* clean up the window */
    SDL_Quit( );

    /* and exit appropriately */
    exit( returnCode );
}

/* function to handle key press events */
void handleKeyPress( SDL_keysym *keysym ) {
    switch ( keysym->sym )
	{
	case SDLK_ESCAPE:
	    /* ESC key was pressed */
	    Quit( 0 );
	    break;
	case SDLK_F1:
	    /* F1 key was pressed
	     * this toggles fullscreen mode
	     */
	    SDL_WM_ToggleFullScreen( surface );
	    break;
	default:
	    break;
	}

    return;
}

void initSDL(const SDL_VideoInfo *& videoInfo, screen_data & screen, int &videoFlags) {
  /* initialize SDL */
  if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
    std::cerr<<"Video initialization failed: "<<SDL_GetError( )<<std::endl;
  }

  /* Fetch the video info */
  videoInfo = SDL_GetVideoInfo( );

  if ( !videoInfo ) {
    std::cerr<<"Video query failed: "<<SDL_GetError( )<<std::endl;
  }

  /* the flags to pass to SDL_SetVideoMode */
  videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL */
  videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
  videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
  videoFlags |= SDL_RESIZABLE;       /* Enable window resizing */

  /* This checks to see if surfaces can be stored in memory */
  if ( videoInfo->hw_available )
    videoFlags |= SDL_HWSURFACE;
  else
    videoFlags |= SDL_SWSURFACE;

  /* This checks if hardware blits can be done */
  if ( videoInfo->blit_hw )
    videoFlags |= SDL_HWACCEL;

  /* Sets up OpenGL double buffering */
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

  /* get a SDL surface */
  surface = SDL_SetVideoMode( screen.pixWidth, screen.pixHeight, SCREEN_BPP,
			      videoFlags );

  /* Verify there is a surface */
  if ( !surface ) {
    std::cerr<<"Video mode set failed: "<<SDL_GetError( )<<std::endl;
    Quit( 1 );
  }
}

/* general OpenGL initialization function */
int initGL( void ) {

    /* Enable smooth shading */
    glShadeModel( GL_SMOOTH );

    /* Set the background black */
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    /* Depth buffer setup */
    glClearDepth( 1.0f );

    /* Enables Depth Testing */
    glEnable( GL_DEPTH_TEST );

    /* The Type Of Depth Test To Do */
    glDepthFunc( GL_LEQUAL );

    /* Really Nice Perspective Calculations */
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    return( 1 );
}

void draw_square() {

  glBegin( GL_QUADS );                 /* Draw A Quad              */
  {
    glVertex3f(  0.9f,  0.9f,  0.0f ); /* Top Right Of The Quad    */
    glVertex3f( -0.9f,  0.9f,  0.0f ); /* Top Left Of The Quad     */
    glVertex3f( -0.9f, -0.9f,  0.0f ); /* Bottom Left Of The Quad  */
    glVertex3f(  0.9f, -0.9f,  0.0f ); /* Bottom Right Of The Quad */
  }
  glEnd( );                       /* Done Drawing The Quad    */
}

int main( int argc, char **argv ) {
    /* Flags to pass to SDL_SetVideoMode */
    int videoFlags;
    /* main loop variable */
    bool done = false;
    /* used to collect events */
    SDL_Event event;
    /* this holds some info about our display */
    const SDL_VideoInfo *videoInfo;
    /* whether or not the window is active */
    bool isActive = true;

    dumb_clock Clock;

    game_grid grid;
    dumb_timer iterateGrid(Clock);
    iterateGrid.how_long(1000);

    screen_data screen;

    //mouse:
    dPair dMouse, mouseOffset;
    iPair iMouse;
    bool bMouseButtonDown = false;
    bool bDragging = false;

    dumb_timer dragTimer(Clock);
    dragTimer.how_long(150);

    //selection
    iPair clicked;

    dTriplet grid_center;

    Camera camera;
    dTriplet cameraDragStart;

    bool paused = false;


    Console io;
    dumb_timer checkConsole(Clock);
    checkConsole.how_long(200);

    parse_consol console(io);
    //start the console interface
    boost::thread thrd( boost::ref(io) );

    initSDL( videoInfo, screen, videoFlags );

    /* initialize OpenGL */
    initGL( );

    /* resize the initial window */
    screen.resize_window(screen.pixWidth, screen.pixHeight );

    //setup the camera
    grid_center.X = (double)grid.Width - 1.0;
    grid_center.Y = (double)grid.Height - 1.0;
    grid_center.Z = 0;

    camera.X = grid_center.X;
    camera.Y = grid_center.Y;
    camera.Z = -30;

    /* wait for events */ 
    while ( !done ) {
      /* handle the events in the queue */
      while ( SDL_PollEvent( &event ) ) {
	if( bMouseButtonDown ) {
	  if(bDragging) {
	    SDL_GetMouseState(&iMouse.X, &iMouse.Y);
	    mouseOffset = pix_to_gl(screen,
				    iMouse,
				    camera.Z);
	    camera.X = cameraDragStart.X + mouseOffset.X - dMouse.X;
	    camera.Y = cameraDragStart.Y + mouseOffset.Y - dMouse.Y;
	  } else bDragging = dragTimer.done();
	}
	switch( event.type ) {
	case SDL_ACTIVEEVENT:
	  /* Something's happend with our focus
	   * If we lost focus or we are iconified, we
	   * shouldn't draw the screen
	   */
	  if ( event.active.gain == 0 )
	    isActive = false;
	  else
	    isActive = true;
	  break;			    
	case SDL_VIDEORESIZE:
	  /* handle resize event */
	  surface = SDL_SetVideoMode( event.resize.w,
				      event.resize.h,
				      16, videoFlags );
	  if ( !surface ) {
	    std::cerr<<"Could not get a surface after resize: "<<SDL_GetError( )<<std::endl;
	    Quit( 1 );
	  }
	  screen.resize_window(event.resize.w, event.resize.h );
	  break;
	case SDL_MOUSEBUTTONDOWN:
	  SDL_GetMouseState(&iMouse.X, &iMouse.Y);
	  dMouse = pix_to_gl(screen,
			     iMouse,
			     camera.Z);
	  bMouseButtonDown = true;
	  cameraDragStart = camera;
	  dragTimer.reset();
	  break;
	case SDL_MOUSEBUTTONUP:
	  //todo: check timer, if the button's been held too long and mouse hasn't moved, do nothing
	  if(!bDragging) {
	    std::cout<<"mouse postion: x"<<dMouse.X<<" y"<<dMouse.Y<<std::endl;
	  }
	  bMouseButtonDown = false;
	  bDragging = false;
	  break;
	case SDL_KEYDOWN:
	  /* handle key presses */
	  handleKeyPress( &event.key.keysym );
	  break;
	case SDL_QUIT:
	  /* handle quit requests */
	  done = true;
	  break;
	default:
	  break;
	}
      }

      /* draw the scene */
      if ( isActive ) {
	/* Clear The Screen And The Depth Buffer */
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glLoadIdentity( );
	glTranslatef(grid_center.X - camera.X, grid_center.Y - camera.Y, camera.Z + 0.1);
	glColor3f( 0.1, 0.9, 0.2 );
	glBegin( GL_TRIANGLES ); {
	  glVertex3f( -0.3, -0.3, 0 );
	  glVertex3f( 0, 0.3 , 0);
	  glVertex3f( 0.3, -0.3, 0);
	}
	glEnd();

	for(int i = 0; i < grid.Height; ++i) {
	  for(int j = 0; j < grid.Width; ++j) {
	    glLoadIdentity( );
	    glTranslatef( ((double)2 * i) - camera.X, ((double)2 * j) - camera.Y, camera.Z );
	    if( clicked.X == i && clicked.Y == j ) {
	      glColor3f(0.5, 0.9, 0.5);
	    } else if( grid(i, j) ) {
	      glColor3f(0.1, 0.1, 0.1);
	    } else {
	      glColor3f(0.9, 0.9, 0.9);
	    }	  
	    draw_square();
	  }
	}
      
	/* Draw it to the screen */
	SDL_GL_SwapBuffers( );
      }

      if( iterateGrid.done() ) {
	iterate(grid);
	iterateGrid.reset();
      }
      if( checkConsole.done() ) {
	console(camera);
      }
      Clock.inc();
    }

    /* clean ourselves up and exit */
    Quit( 0 );

    /* Should never get here */
    return( 0 );
}
