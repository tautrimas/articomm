//     This file is part of ARTIcomm.
//
//     ARTIcomm is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     ARTIcomm is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with ARTIcomm.  If not, see <http://www.gnu.org/licenses/>.

// g++ animateagent.cpp -o animateagent -lX11 -lXi -lXmu -lglut -lGL -lGLU -lm -O2 && chmod u+x animateagent && time ./animateagent
// This code was originally created by Jeff Molofee '99 (ported to Linux/GLUT by Richard Campbell '99)

#include <GL/glut.h>    // Header File For The GLUT Library
#include <GL/gl.h>	// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library
#include <GL/glx.h>     // Header file fot the glx libraries.
#include <unistd.h>     // needed to sleep
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <fstream>

using namespace std;
/* ASCII code for the escape key. */
#define ESCAPE 27
#define deginrad (3.1415927/180)
#define rotx 50
#define roty -30
#define rotz 0
#define transx -0.8
#define transy -0.3
#define transz -2.1
#define t (0.028) // /x is for slowing and fastening things up
/* The number of our GLUT window */
int window;

GLuint base; // base display list for the font set.

/* lighting on/off (1 = on, 0 = off) */
int light;

/* rotation angle for the triangle. */
double rtri = 0;

/* rotation angle for the quadrilateral. */
double rquad = 90.62;

/* white ambient light at half intensity (rgba) */
GLfloat LightAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };

/* super bright, full intensity diffuse light. */
GLfloat LightDiffuse[] = { 0.3, 0.3, 0.3, 1 };

/* position of light (x, y, z, (position of light)) */
GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };

#define N 128
double rob[N][3][3];
#define S 100
double sienos[S][4];

bool pirmaskartas = true;

ifstream gin("coords.txt");

int robotCount, fireCount;
double fires[10][2];
double x, y, rot;

void pakraunam()
{
  ifstream fin("robotukas.raw");
  int i=0;
  do
  {
    for (int j=0; j<3; j++)
    {
      fin >> rob[i][j][0];
      fin >> rob[i][j][2];
      fin >> rob[i][j][1];
    }
    i++;
  } while (!fin.eof());
  fin.close();

  fin.open("walls.txt");
  int walls;
  double garbage;
  fin >> garbage >> garbage >> garbage >> garbage >> walls;
  for (i = 0; i < walls; ++i)
  {
    fin >> sienos[i][0];
    fin >> sienos[i][1];
    fin >> sienos[i][2];
    fin >> sienos[i][3];
  }
  fin.close();
  double a, b;
  gin >> a >> b;
  robotCount = a;
  fireCount = b;
  for (int i = 0; i < fireCount; ++i)
  {
    gin >> fires[i][0] >> fires[i][1];
  }
}

void BuildFont()
{
  Display *dpy;
  XFontStruct *fontInfo; // storage for our font.

  base = glGenLists(96); // storage for 96 characters.

  // load the font.  what fonts any of you have is going
  // to be system dependent, but on my system they are
  // in /usr/X11R6/lib/X11/fonts/*, with fonts.alias and
  // fonts.dir explaining what fonts the .pcf.gz files
  // are.  in any case, one of these 2 fonts should be
  // on your system...or you won't see any text.

  // get the current display.  This opens a second
  // connection to the display in the DISPLAY environment
  // value, and will be around only long enough to load
  // the font.
  dpy = XOpenDisplay(NULL); // default to DISPLAY env.

  fontInfo = XLoadQueryFont(dpy,
      "-adobe-helvetica-medium-r-normal--18-*-*-*-p-*-iso8859-1");
  if (fontInfo == NULL)
  {
    fontInfo = XLoadQueryFont(dpy, "fixed");
    if (fontInfo == NULL)
    {
      printf("no X font available?\n");
    }
  }

  // after loading this font info, this would probably be the time
  // to rotate, scale, or otherwise twink your fonts.

  // start at character 32 (space), get 96 characters (a few characters past z), and
  // store them starting at base.
  glXUseXFont(fontInfo->fid, 32, 96, base);

  // free that font's info now that we've got the
  // display lists.
  XFreeFont(dpy, fontInfo);

  // close down the 2nd display connection.
  XCloseDisplay(dpy);
}

void KillFont() // delete the font.
{
  glDeleteLists(base, 96); // delete all 96 characters.
}

void glPrint(char *text) // custom gl print routine.
{
  if (text == NULL)
  { // if there's no text, do nothing.
    return;
  }

  glPushAttrib(GL_LIST_BIT); // alert that we're about to offset the display lists with glListBase
  glListBase(base - 32); // sets the base character to 32.

  glCallLists(strlen(text), GL_UNSIGNED_BYTE, text); // draws the display list text.
  glPopAttrib(); // undoes the glPushAttrib(GL_LIST_BIT);
}

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
void InitGL(int Width, int Height) // We call this right after our OpenGL window is created.
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // This Will Clear The Background Color To Black
  glClearDepth(1.0); // Enables Clearing Of The Depth Buffer
  glDepthFunc(GL_LESS); // The Type Of Depth Test To Do
  glEnable(GL_DEPTH_TEST); // Enables Depth Testing
  glShadeModel(GL_SMOOTH); // Enables Smooth Color Shading

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity(); // Reset The Projection Matrix

  gluPerspective(45.0f, (GLdouble)Width/(GLdouble)Height, 0.1f, 100.0f); // Calculate The Aspect Ratio Of The Window

  glMatrixMode(GL_MODELVIEW);

  glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient); // add lighting. (ambient)
  glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse); // add lighting. (diffuse).
  glLightfv(GL_LIGHT1, GL_POSITION,LightPosition); // set light position.
  glEnable(GL_LIGHT1); // turn light 1 on.
  BuildFont();
}

/* The function called when our window is resized (which shouldn't happen, because we're fullscreen) */
void ReSizeGLScene(int Width, int Height)
{
  if (Height==0) // Prevent A Divide By Zero If The Window Is Too Small
    Height=1;

  glViewport(0, 0, Width, Height); // Reset The Current Viewport And Perspective Transformation

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(45.0f, (GLdouble)Width/(GLdouble)Height, 0.1f, 100.0f);
  glMatrixMode(GL_MODELVIEW);
}

//http://www.cs.fiu.edu/~weiss/dsaa_c2e/fig10_38.c
void msum(double A[][4], double B[][4])
{
  int i, j, k;
  double C[4][4];
  for (i = 0; i < 4; i++)
    /* Initialization */
    for (j = 0; j < 4; j++)
      C[ i ][ j ] = 0.0;

  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++)
      for (k = 0; k < 4; k++)
        C[ i ][ j ] += A[ i ][ k ] * B[ k ][ j ];

  for (i=0; i<4; i++)
    for (j=0; j<4; j++)
      A[i][j]=C[i][j];
}

void mvien(double b[][4])
{
  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++)
      b[i][j] = 0;
  b[0][0] = 1;
  b[1][1] = 1;
  b[2][2] = 1;
  b[3][3] = 1;
}

void mrotx(double a[][4], double r)
{
  double b[4][4];
  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++)
      b[i][j] = 0;
  r *= deginrad;
  b[0][0] = 1;
  b[1][1] = cos(r);
  b[1][2] = sin(r);
  b[2][1] = -sin(r);
  b[2][2] = cos(r);
  b[3][3] = 1;
  msum(a, b);
}

void mroty(double a[][4], double r)
{
  double b[4][4];
  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++)
      b[i][j] = 0;
  r *= deginrad;
  b[0][0] = cos(r);
  b[1][1] = 1;
  b[0][2] = -sin(r);
  b[2][0] = sin(r);
  b[2][2] = cos(r);
  b[3][3] = 1;
  msum(a, b);
}

void mrotz(double a[][4], double r)
{
  double b[4][4];
  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++)
      b[i][j] = 0;
  r *= deginrad;
  b[0][0] = cos(r);
  b[0][1] = sin(r);
  b[1][0] = -sin(r);
  b[1][1] = cos(r);
  b[2][2] = 1;
  b[3][3] = 1;
  msum(a, b);
}

void mtrans(double a[][4], double x, double y, double z)
{
  double b[4][4];
  mvien(b);
  b[3][0] = x;
  b[3][1] = y;
  b[3][2] = z;
  msum(a, b);
}

void vsum(double a[][4], double &x, double &y, double &z)
{
  double x1=x, y1=y, z1=z;
  x = a[0][0] * x1 + a[1][0] * y1 + a[2][0] * z1 + a[3][0];
  y = a[0][1] * x1 + a[1][1] * y1 + a[2][1] * z1 + a[3][1];
  z = a[0][2] * x1 + a[1][2] * y1 + a[2][2] * z1 + a[3][2];
}

void siena(double x1, double y1, double x2, double y2, double spalva)
{
  glBegin(GL_POLYGON);
  glColor3f(spalva, 0, 0);
  double z=(cos(rtri*5)+1)/200+0.03;
  double mat[4][4];
  double vert[4][3] = { { x1, 0, -y1 }, { x2, 0, -y2 }, { x2, z, -y2 }, { x1,
      z,
      -y1 } };
  //printf("%0.1f %0.1f ",x1,y1);
  for (int i=0; i<4; i++)
  {
    mvien(mat);
    mrotx(mat, rotx);
    mroty(mat, roty);
    mrotz(mat, rotz);
    mtrans(mat, transx, transy, transz);
    vsum(mat, vert[i][0], vert[i][1], vert[i][2]);
    glVertex3f(vert[i][0], vert[i][1], vert[i][2]);
  }
  //printf("%0.1f %0.1f %0.1f\n",vert[0][0],vert[0][1],vert[0][2]);
  glEnd();
}

void tekstas()
{
  double base1, base2, hung, tired, azimuth, elapsed;
  gin >> base1 >> base2;//>> tired >> azimuth >> elapsed;
  glLoadIdentity();
  glTranslatef(0.0f, 0.0f, -1.0f); // move 1 unit into the screen.

  glColor3f(1.0f, 1.0f, 0.0f);
  double defx = -0.5000f, defy = 0.38000f, defstep = 0.018f;
  int i=0;
  char buffer [50];
  int n;

  if (base1 > 0.0)
    glColor3f(1.0f, 0.0f, 0.0f);
  glRasterPos2f(defx, defy-defstep*i++);
  n=sprintf(buffer, "Eating: %0.1f", base1);
  glPrint(buffer);
  glColor3f(1.0f, 1.0f, 0.0f);

  /*if (base2 > 0.0) glColor3f(1.0f,0.0f,0.0f);
   glRasterPos2f(defx,defy-defstep*i++);
   n=sprintf (buffer, "Sleeping: %0.1f", base2);
   glPrint(buffer);
   glColor3f(1.0f,1.0f,0.0f);

   if (hung > tired) glColor3f(0.0f,1.0f,0.0f);
   glRasterPos2f(defx,defy-defstep*i++);
   n=sprintf (buffer, "Hunger: %0.1f", hung);
   glPrint(buffer);
   glColor3f(1.0f,1.0f,0.0f);

   if (hung < tired) glColor3f(0.0f,1.0f,0.0f);
   glRasterPos2f(defx,defy-defstep*i++);
   n=sprintf (buffer, "Sleep: %0.1f", tired);
   glPrint(buffer);

   glColor3f(1.0f,1.0f,0.0f);
   glRasterPos2f(defx,defy-defstep*i++);
   n=sprintf (buffer, "Azimuth: %0.0f", azimuth);
   glPrint(buffer);

   glRasterPos2f(defx,defy-defstep*i++);
   n=sprintf (buffer, "Time elapsed: %0.1f", elapsed);
   glPrint(buffer);*/
}
/* The main drawing function. */
void DrawGLScene()
{
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); // Clear The Screen And The Depth Buffer
  glLoadIdentity(); // Reset The View
  //glRotatef(rtri,1.0,0,0);
  rtri += 0.1;

  for (int j=0; j<S; j++)
    siena(sienos[j][0], sienos[j][1], sienos[j][2], sienos[j][3], 0.8-(j%2)*0.1);

  glLoadIdentity();
  glBegin(GL_POLYGON);
  glColor3f(0.9, 0.9, 0.9);
  double mat[4][4];
  double vertex[3];
  double floor[4*2]= { -0.2, 1.1, 1.6, 1.1, 1.6, -0.2, -0.2, -0.2 };
  for (int i=0; i<4; i++)
  {
    if (i==2)
      glColor3f(0.7, 0.7, 0.7);
    mvien(mat);
    mrotx(mat, rotx);
    mroty(mat, roty);
    mrotz(mat, rotz);
    mtrans(mat, transx, transy, transz);
    vertex[0] = floor[i*2];
    vertex[1] = 0.0;
    vertex[2] = -floor[i*2+1];
    //printf("%f %f %f\n", vertex[0],vertex[1],vertex[2]);
    vsum(mat, vertex[0], vertex[1], vertex[2]);
    //printf("%f %f %f\n", vertex[0],vertex[1],vertex[2]);
    glVertex3f(vertex[0], vertex[1], vertex[2]);
  }
  glEnd();
  /**********************************
   Robotuko piešimas
   **********************************/
  for (int nr = 0; nr < robotCount; ++nr)
  {
    glLoadIdentity();
    gin >> x >> y >> rot;
    glColor3f(0, 0.3, 0.3);
    for (int i=0; i<N; i++)
    {
      glBegin(GL_TRIANGLES);
      //printf("%0.1f %0.1f ",x1,y1);
      for (int j=0; j<3; j++)
      {
        if (i==64 || i==65)
          glColor3f(1, 1, 0);
        else if (i>=66)
          glColor3f(0, (i-64)/64.0, (i-64)/64.0);
        mvien(mat);
        mroty(mat, rot+45);
        mtrans(mat, x, 0, -y);
        mrotx(mat, rotx);
        mroty(mat, roty);
        mrotz(mat, rotz);
        mtrans(mat, transx, transy, transz);
        for (int k=0; k<3; k++)
        {
          vertex[k] = rob[i][j][k];
        }
        vsum(mat, vertex[0], vertex[1], vertex[2]);
        glVertex3f(vertex[0], vertex[1], vertex[2]);
      }
      glEnd();
    }
    // tekstas();
  }
  for (int i = 0; i < fireCount; ++i)
  {
    gin >> rot;
    glLoadIdentity();
    glBegin(GL_TRIANGLES);
    glColor3f(1.0, 0.282, 0.0);
    for (int j = 0; j < 3; ++j)
    {
      mvien(mat);
      mroty(mat, j * 120 + rtri*10);
      mtrans(mat, fires[i][0], 0.01, -fires[i][1]);
      mrotx(mat, rotx);
      mroty(mat, roty);
      mrotz(mat, rotz);
      mtrans(mat, transx, transy, transz);
      vertex[0] = 0.0;
      vertex[1] = 0.0;
      vertex[2] = -rot;
      vsum(mat, vertex[0], vertex[1], vertex[2]);
      glVertex3f(vertex[0], vertex[1], vertex[2]);
    }
    glEnd();
  }

  // swap the buffers to display, since double buffering is used.
  glutSwapBuffers();
  if (pirmaskartas)
  {
    usleep(1000000);
    pirmaskartas = false;
  }
  if (gin.eof())
  {
    usleep(1000000);
    KillFont();
    exit(0);
  }
  usleep(useconds_t(t*1000000));
  //exit(0);
}

/* The function called whenever a key is pressed. */
void keyPressed(unsigned char key, int x, int y)
{
  /* avoid thrashing this call */
  usleep(100);

  /* If escape is pressed, kill everything. */
  switch (key)
  {
    case ESCAPE:
      KillFont();
      /* shut down our window */
      glutDestroyWindow(window);

      /* exit the program...normal termination. */
      exit(0);
      break;
    case 108:
      light = light ? 0 : 1; // switch the current value of light, between 0 and 1.
      if (!light)
      {
        glDisable(GL_LIGHTING);
      }
      else
      {
        glEnable(GL_LIGHTING);
      }
      break;
  }
}

int main(int argc, char **argv)
{
  pakraunam();
  /* Initialize GLUT state - glut will take any command line arguments that pertain to it or
   X Windows - look at its documentation at http://reality.sgi.com/mjk/spec3/spec3.html */
  glutInit(&argc, argv);

  /* Select type of Display mode:
   Double buffer
   RGBA color
   Alpha components supported
   Depth buffered for automatic clipping */
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);

  /* get a 640 x 480 window */
  glutInitWindowSize(800, 600);

  /* the window starts at the upper left corner of the screen */
  glutInitWindowPosition(0, 0);

  /* Open a window */
  window = glutCreateWindow("AnnEVO");

  /* Register the function to do all our OpenGL drawing. */
  glutDisplayFunc(&DrawGLScene);

  /* Go fullscreen.  This is as soon as possible. */
  glutFullScreen();

  /* Even if there are no events, redraw our gl scene. */
  glutIdleFunc(&DrawGLScene);

  /* Register the function called when our window is resized. */
  glutReshapeFunc(&ReSizeGLScene);

  /* Register the function called when the keyboard is pressed. */
  glutKeyboardFunc(&keyPressed);

  /* Initialize our window. */
  InitGL(800, 600);

  //usleep(1000000);
  /* Start Event Processing Engine */
  glutMainLoop();

  return 1;
}
