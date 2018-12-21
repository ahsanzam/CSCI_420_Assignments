// /*
//   CSCI 420 Computer Graphics
//   Assignment 1: Height Fields
//   C++ starter code
//   Ahsan Zaman
//   September 24, 2018
// */

#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>
#include <cmath>
#include <iostream>
using namespace std;

bool record = false;
char * TEXTURE_FILENAME = "extras/grass_texture.jpg";
string color_filename = "color";

int g_iMenuId;
int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
int renderMode = GL_POINT;
bool overlay = false, texture_on=false;
CONTROLSTATE g_ControlState = ROTATE;
GLuint texture[1];
/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};
float window_width = 0.0;
float window_height = 0.0;
float angle_f = 0; //used for autorotate function
bool auto_rotate = false, auto_scale=false, color_image=true, color_on=false, scaleUp = true;
double auto_scale_val = 0.0;
double time_since_last_screenshot = 0;
char * myFilenm = new char[25];
int screenShotnum = 0;

/* see <your pic directory>/pic.h for type Pic */
Pic * g_pHeightData, *g_colorData;
float max_value = 255.0;

/* Write a screenshot to the specified filename */
void saveScreenshot (char *filename){
  int i, j;
  Pic *in = NULL;
  if (filename == NULL) return;
  in = pic_alloc(640, 480, 3, NULL); //Allocate a picture buffer
  // printf("File to save to: %s\n", filename);
  for (i=479; i>=0; i--)
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                 &in->pix[i*in->nx*in->bpp]);
  jpeg_write(filename, in);
  // if (jpeg_write(filename, in))
  //   printf("File saved Successfully\n");
  // else
  //   printf("Error in Saving\n");
  pic_free(in);
}

void myinit(){
  /* setup gl view here */
  // GL_LIGHT GL_LIGHT0;
  // GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  // GLfloat mat_shininess[] = { 0.0 };
  // GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
  // glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  // glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  // glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  // glEnable(GL_LIGHTING);
  // glEnable(GL_COLOR_MATERIAL);
  // glEnable(GL_LIGHT0);

  glClearColor(0.0,0.0,0.0,0.0);//0.9098, 0.945098, 0.9490196, 0.0);
  glEnable(GL_DEPTH_TEST);            // enable depth buffering
  glShadeModel(GL_SMOOTH);            // interpolate colors during rasterization
  glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
  glPolygonOffset(0.0, 1.0); //slightly offset points at the same x,y,z coordinates
}

float max(float a, float b){
  return (a > b) ? a : b;
}
void rotateTo(float x, float y, float z){
  glRotatef(angle_f/10, x, 0.0, 0.0);
  glRotatef(angle_f/100.00, 0.0, y, 0.0);
  glRotatef(angle_f, 0.0, 0.0, z);
  if(auto_rotate) angle_f += 1;
}
void loadTextureFromFile()
{   
  glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_FLAT);
  glEnable(GL_DEPTH_TEST);

  Pic * texture_pic = jpeg_read(TEXTURE_FILENAME, NULL); //read in the texture file
  glGenTextures(1, &texture[0]);               // Create The Texture
  glBindTexture(GL_TEXTURE_2D, texture[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  // texture generation
  glTexImage2D(GL_TEXTURE_2D, 0, 3, texture_pic->nx, texture_pic->ny, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_pic->pix);
}
void do_record(){
  if(((float)(clock()- time_since_last_screenshot)/CLOCKS_PER_SEC) >= (1/30) && screenShotnum < 300){
    sprintf(myFilenm, "screenshots/%03d.jpg", screenShotnum); 
    saveScreenshot(myFilenm); 
    screenShotnum += 1;
    time_since_last_screenshot = clock();
  }
  if(screenShotnum == 299){
    printf("Finished recording.\n");
    auto_rotate = false;
    record = false;
  }
  else if(screenShotnum == 1){
   auto_rotate = true;
   auto_scale = true;
  }
  if(screenShotnum == 40){
    renderMode = GL_LINE;    
  }
  if(screenShotnum == 40*2){
    renderMode = GL_FILL;    
    auto_scale = true;
  }
  if(screenShotnum == 40*3)
    overlay=true;    
  if(screenShotnum == 40*4)
    overlay=false;    
  if(screenShotnum == 40*5 && color_image)
    color_on = true;  
  if(screenShotnum == 40*6)
    color_on = false;  
}

void display(){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity(); // reset transformation
  
  //transformations through modifiers shift, ctrl/alt, and default rotate 
  glTranslatef(g_vLandTranslate[0],g_vLandTranslate[1],-15+g_vLandTranslate[2]);
  glScalef(g_vLandScale[0]+auto_scale_val, g_vLandScale[1]+auto_scale_val, g_vLandScale[2]);
  glRotatef(sqrt(pow(g_vLandRotate[0],2)+pow(g_vLandRotate[1],2)), g_vLandRotate[0], g_vLandRotate[1], g_vLandRotate[2]);

  glRotatef(180,0,0,1);

  rotateTo(.001, .001, .001); //autorotate call

  if(overlay){ //overlay function 
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); //sets up the wireframe mode
    glColor4f(1.0, 0.0, 0.0, 1.0); //should be a red color wireframe
    for(int i=0; i<g_pHeightData->nx-1; i++){ 
      glBegin(GL_TRIANGLE_STRIP);
        for(int j=0; j<g_pHeightData->ny; j++){ //goes down the columns and creates a triangle strip
          float height_1 = .5*(float)PIC_PIXEL(g_pHeightData, i, j, 0) / max_value;
          float height_2 = .5*(float)PIC_PIXEL(g_pHeightData, i+1, j, 0) / max_value;
          glVertex3f( -4*(((float)(i+1)/(float)g_pHeightData->nx)-.5),
                      4*(((float)j/(float)g_pHeightData->ny)-.5),
                      height_2 );
          glVertex3f( -4*(((float)i/(float)g_pHeightData->nx)-.5),
                      4*(((float)j/(float)g_pHeightData->ny)-.5),
                      height_1 );
        }
      glEnd();
    }
  }

  if(texture_on){ //render textures if desired
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
  }
  //draw the object
  glPolygonMode( GL_FRONT_AND_BACK, renderMode );
  for(int i=0; i<g_pHeightData->nx-1; i++){
    glBegin(GL_TRIANGLE_STRIP);
      for(int j=0; j<g_pHeightData->ny; j++){
        //calculate the height of the bottom and top vertices
        float height_1 = .5*(float)PIC_PIXEL(g_pHeightData, i, j, 0) / max_value;
        float height_2 = .5*(float)PIC_PIXEL(g_pHeightData, i+1, j, 0) / max_value;
        if(texture_on) //render textures if desired
          glTexCoord2f( i, j );
        if(color_on) //render color from other picture if desired
          glColor4f(2*height_1*(float)PIC_PIXEL(g_colorData, i, j, 0)/255.0, 
                    2*height_1*(float)PIC_PIXEL(g_colorData, i, j, 1)/255.0, 
                    2*height_1*(float)PIC_PIXEL(g_colorData, i, j, 2)/255.0,
                    1);
        else glColor4f(2*height_1, 2*height_1, 1, 1); 
        //render vertex
        glVertex3f( -4*(((float)i/(float)g_pHeightData->nx)-.5),
                    4*(((float)j/(float)g_pHeightData->ny)-.5),
                    height_1 );
        if(color_on) //render color from other picture if desired
          glColor4f(2*height_2*(float)PIC_PIXEL(g_colorData, i, j, 0)/255.0, 
                    2*height_2*(float)PIC_PIXEL(g_colorData, i, j, 1)/255.0, 
                    2*height_2*(float)PIC_PIXEL(g_colorData, i, j, 2)/255.0,
                    1);
        else glColor4f(2*height_2, 2*height_2, 1, 1);
        //render vertex
        glVertex3f( -4*(((float)(i+1)/(float)g_pHeightData->nx)-.5),
                    4*(((float)j/(float)g_pHeightData->ny)-.5),
                    height_2 );
      }
    glEnd();
  }
  if(texture_on) //render textures if desired
    glDisable(GL_TEXTURE_2D);

  glutSwapBuffers(); // double buffer flush

  if(record) do_record();
}

void menufunc(int value){
  switch (value){
    case 0: //points
      if(!overlay && !texture_on) 
        renderMode = GL_POINT;
      break;
    case 1: //wireframe
      if(!overlay && !texture_on) 
        renderMode = GL_LINE;
      break;
    case 2: //solid
      renderMode = GL_FILL;
      break;
    case 3: //toggle wireframe overlay
      overlay = !overlay;
      if(overlay) renderMode = GL_FILL;
      break;
    case 4: //toggle autorotate
      auto_rotate = !auto_rotate;
      break; 
    case 5: //toggle texture
      texture_on = !texture_on;
      if(texture_on) renderMode = GL_FILL;
      break;
    case 6: //exit or toggle color
      if(color_image){
        color_on = !color_on;
        break;
      }
      else exit(0);
    case 7: //exit
      exit(0);
  }
}

void doIdle(){
  glutPostRedisplay();
}

/* converts mouse drags into information about 
rotation/translation/scaling */
void mousedrag(int x, int y){
  int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
  switch (g_ControlState){
    case TRANSLATE:  
      if (g_iLeftMouseButton){
        g_vLandTranslate[0] += vMouseDelta[0]*0.01;
        g_vLandTranslate[1] -= vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton){
        g_vLandTranslate[2] += vMouseDelta[1]*0.01;
      }
      break;
    case ROTATE:
      if (g_iLeftMouseButton){
        g_vLandRotate[0] += vMouseDelta[1];
        g_vLandRotate[1] += vMouseDelta[0];
      }
      if (g_iMiddleMouseButton){
        g_vLandRotate[2] += vMouseDelta[1];
      }
      break;
    case SCALE:
      if (g_iLeftMouseButton){
        g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
        g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton){
        g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
      }
      break;
  }
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mouseidle(int x, int y){
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y){
  switch (button){
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
  }
  switch(glutGetModifiers()){
    case GLUT_ACTIVE_CTRL: //this is an issue on mac since ctrl+click=right click.
      g_ControlState = TRANSLATE;
      break;
    case GLUT_ACTIVE_ALT: //for mac users ALT DOESN'T WORK EITHER!!
      g_ControlState = TRANSLATE;
      break;
    case GLUT_ACTIVE_SHIFT:
      g_ControlState = SCALE;
      break;
    default:
      g_ControlState = ROTATE;
      break;
  }

  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void keyDown (unsigned char key, int x, int y) { 
  switch(key){
    case 't':
      g_ControlState = TRANSLATE;
      break;
    case 'T':
      g_ControlState = TRANSLATE;
      break;
    case 's':
      g_ControlState = SCALE;
      break;
    case 'S':
      g_ControlState = SCALE;
      break;
  }
}

void keyUp(unsigned char key, int x, int y){
  g_ControlState = ROTATE;
}

// called every time window is resized to update projection matrix
void reshape(int w, int h){
  glMatrixMode(GL_MODELVIEW);
  // setup image size
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  // setup camera
  glFrustum(-0.1, 0.1, -float(h)/(10.0*float(w)), float(h)/(10.0*float(w)), 0.5, 1000.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  window_width = glutGet(GLUT_WINDOW_WIDTH);
  window_height = glutGet(GLUT_WINDOW_HEIGHT);
}

int main (int argc, char ** argv){
  if (argc<2){  
    printf ("usage: %s heightfield.jpg\n", argv[0]);
    exit(1);
  }

  g_pHeightData = jpeg_read(argv[1], NULL);
  if (!g_pHeightData){
    printf ("error reading %s.\n", argv[1]);
    exit(1);
  }
  //read in the color data file
  string color_file = "extras/"+color_filename+to_string(g_pHeightData->nx)+".jpg";
  char* color_file_char = new char[color_file.size()+1];
  std::copy(color_file.begin(), color_file.end(), color_file_char);
  color_file_char[color_file.size()] = '\0';
  g_colorData = jpeg_read(color_file_char, NULL);

  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA); // request double buffer
  glutInitWindowSize(640,480); // set window size
  glutInitWindowPosition(0, 0); // set window position
  glutCreateWindow("Height Field"); // creates a window

  /* allow the user to quit using the right mouse button menu */
  g_iMenuId = glutCreateMenu(menufunc);
  glutSetMenu(g_iMenuId);
  glutAddMenuEntry("Points",0);
  glutAddMenuEntry("Wireframe",1);
  glutAddMenuEntry("Solid",2);
  glutAddMenuEntry("Overlay",3);
  glutAddMenuEntry("Autorotate",4);
  glutAddMenuEntry("Texture",5);
  if(!g_colorData){
    printf("Color file not found. Color option will be disabled.\n");
    color_image = false;
    glutAddMenuEntry("Quit",6);
  }
  else{
    glutAddMenuEntry("Color Mapping",6);
    glutAddMenuEntry("Quit",7);
  } 
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  loadTextureFromFile();

  glutIdleFunc(doIdle); //animation
  glutMotionFunc(mousedrag); //mouse drags
  glutPassiveMotionFunc(mouseidle); //idle mouse
  glutMouseFunc(mousebutton); //mouse buttons
  glutKeyboardFunc(keyDown);
  glutKeyboardUpFunc(keyUp);
  myinit();

  glutReshapeFunc(reshape); //reshape func
  glutDisplayFunc(display); //display func
  time_since_last_screenshot = clock();
  glutMainLoop(); //start program ui

  return(0);
}