/*
  CSCI 480
  Assignment 2
  Author: Ahsan Zaman
  Email: ahsanzam@usc.edu
  Date: October 18, 2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include "pic.h"
using namespace std;
/************************/
bool testing=false; //testing mode
bool record=false; //save first 1000 screenshots
double MOVE_INTERVAL = 10, TRACK_INTERVAL=3; //modifies the movement speed and slat spacing for different track sizes
// bool renderHeightmap = false; //render heightmap; it looks weird without lighting so I turned it off
/************************/

int g_vMousePos[2] = {0, 0};
float g_vLandRotate[3] = {0.0, 0.0, 0.0}, g_vLandTranslate[3] = {0.0, 0.0, 0.0}, g_vLandScale[3] = {1.0, 1.0, 1.0};
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
CONTROLSTATE g_ControlState = ROTATE;
int g_iMiddleMouseButton = 0, g_iRightMouseButton = 0, g_iLeftMouseButton = 0, g_iMenuId;;
char * myFilenm = new char[25];
int screenShotnum = 0, currPoint = 0, numTextures=8;
float window_width = 0.0, window_height = 0.0;
double timeSinceLastScreenshot, timeSinceLastMove, FRAMES_PER_SECOND=25, U_INTERVAL=.001;
string skyboxFilepaths[8] = { "Skybox/Front",
                              "Skybox/Right",
                              "Skybox/Left",
                              "Skybox/Up",
                              "Skybox/Back",
                              "Skybox/Down",
                              "woodTexture", //used for the slats
                              "steelTexture"}; //used for the bottom and rails
GLuint textures[8]; //holds the different loaded textures
struct point { double x; double y; double z; }; //represents one control point along the spline
/* spline struct which contains how many control points, and an array of control points */
struct spline { int numControlPoints; struct point *points; };
struct spline *g_Splines; //the spline array
int g_iNumOfSplines; //total number of splines
vector<point> trackPoints, upVector, lookAt, sideVector; //holds the calculated spline points
Pic * g_pHeightData, *g_colorData; //holds height map for floor
float max_value = 255.0;
GLuint listIndex; //I used glu lists for fast drawing of tracks and skybox
GLubyte lists[10]; 

/* Write a screenshot to the specified filename */
void saveScreenshot(char *filename){
  Pic *in = NULL;
  if (filename == NULL) return;
  in = pic_alloc(640, 480, 3, NULL); //Allocate a picture buffer
  for (int i=479; i>=0; i--)
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE, &in->pix[i*in->nx*in->bpp]);
  jpeg_write(filename, in);
  pic_free(in);
}
int loadSplines(char *argv) {
  char *cName = (char *)malloc(128 * sizeof(char));
  FILE *fileList;
  FILE *fileSpline;
  int iType, j, iLength, i = 0;

  /* load the track file */
  fileList = fopen(argv, "r");
  if (fileList == NULL) {
    printf ("can't open file\n");
    exit(1);
  }
  
  /* stores the number of splines in a global variable */
  fscanf(fileList, "%d", &g_iNumOfSplines);

  g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

  /* reads through the spline files */
  for (j = 0; j < g_iNumOfSplines; j++) {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL) {
      printf ("can't open file\n");
      exit(1);
    }

    /* gets length for spline file */
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    /* allocate memory for all the points */
    g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
    g_Splines[j].numControlPoints = iLength;

    /* saves the data to the struct */
    while (fscanf(fileSpline, "%lf %lf %lf", 
     &g_Splines[j].points[i].x, 
     &g_Splines[j].points[i].y, 
     &g_Splines[j].points[i].z) != EOF) {
      i++;
    }
  }

  free(cName);

  return 0;
}
void myinit(){
  /* setup gl view here */
  glClearColor(31.0/255,45.0/255,71.0/255,0.0);
  // glEnable(GL_DEPTH_TEST);           // enable depth buffering
  glShadeModel(GL_SMOOTH);            // interpolate colors during rasterization
  glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
  glPolygonOffset(0.0, 1.0); //slightly offset points at the same x,y,z coordinatesk
}
char* convertStringToChar(string s){
  char* s_char = new char[s.size()+1];
  std::copy(s.begin(), s.end(), s_char);
  s_char[s.size()] = '\0';
  return s_char;
}
void initTextures(){
  for(int textureID=0; textureID<numTextures; textureID++){
    string textureFile = "./assets/"+skyboxFilepaths[textureID]+".jpg";
    char* textureFile_char = convertStringToChar(textureFile);
    Pic *texture_pic = jpeg_read(textureFile_char, NULL); //read in the texture file
    if(texture_pic == NULL){
      cout << "Error opening texture file "<<"./assets/"+skyboxFilepaths[textureID]<<".jpeg" << endl;
      exit(2);
    }
    // texture generation
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glGenTextures(1, &textures[textureID]); // Create The Texture
    glBindTexture(GL_TEXTURE_2D, textures[textureID]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_pic->nx, texture_pic->ny, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_pic->pix);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
}
double* indexArr(double *arr, int posx, int posy, int sizey){
  return &*(arr+posx*sizey+posy);
}
void matrix_multiply(double *a, int ax, int ay, double *b, int bx, int by, double *c){
  for(int i=0; i<ax; i++){
    for(int j=0; j<by; j++){
      *indexArr(c, i, j, by) = 0;
      for(int k=0; k<ay; k++)
        *indexArr(c, i, j, by) += *indexArr(a, i, k, ay) * *indexArr(b, k, j, by);
    }
  }
}
void cross_prod(point a,double *b,double *c){
  c[0] = a.y*b[2]-a.z*b[1];
  c[1] = a.z*b[0]-a.x*b[2];
  c[2] = a.x*b[1]-a.y*b[0];
}
void cross_prod(point a,point b,double *c){
  c[0] = a.y*b.z-a.z*b.y;
  c[1] = a.z*b.x-a.x*b.z;
  c[2] = a.x*b.y-a.y*b.x;
}
void cross_prod(double *a,point b,double *c){
  c[0] = a[1]*b.z-a[2]*b.y;
  c[1] = a[2]*b.x-a[0]*b.z;
  c[2] = a[0]*b.y-a[1]*b.x;
}
double norm(double a, double b, double c){
  return pow(pow(a,2)+pow(b,2)+pow(c,2),.5);
}
void calculateSplines(){
  double s = .5;
  double mm[4][3], c[1][3], c_upPoints[3], c_lookAt[3], c_sidePoints[3], mm2[4][3], c_prime[1][3];
  double catmullRom_basis [4][4] ={ {-s,  2-s,  s-2,   s },
                                    {2*s, s-3,  3-2*s, -s},
                                    {-s,  0,    s,     0 },
                                    {0,   1,    0,     0 } };
  for(int i=0; i<g_iNumOfSplines; i++){
    for(int j=0; j<g_Splines[i].numControlPoints-3; j++){
    double control_matrix [4][3] = {  {g_Splines[i].points[j  ].x,  g_Splines[i].points[j  ].y,    g_Splines[i].points[j  ].z},
                                      {g_Splines[i].points[j+1].x,  g_Splines[i].points[j+1].y,    g_Splines[i].points[j+1].z},
                                      {g_Splines[i].points[j+2].x,  g_Splines[i].points[j+2].y,    g_Splines[i].points[j+2].z},
                                      {g_Splines[i].points[j+3].x,  g_Splines[i].points[j+3].y,    g_Splines[i].points[j+3].z} };
      for(double u=0; u<=1; u+=U_INTERVAL){
        //position vector
        double u_array [1][4] = {{pow(u,3),pow(u,2),u,1}};
        matrix_multiply(&(catmullRom_basis[0][0]), 4, 4, &(control_matrix[0][0]), 4, 3, &(mm[0][0]));
        matrix_multiply(&(u_array[0][0]), 1, 4, &(mm[0][0]), 4, 3, &(c[0][0]));
        point points = {c[0][0], c[0][1], c[0][2]};
        trackPoints.push_back(points);
        //Tangent vector
        double u_der_array [1][4] = {{3*pow(u,2), 2*u, 1, 0}};
        matrix_multiply(&(u_der_array[0][0]), 1, 4, &(mm[0][0]), 4, 3, &(c_lookAt[0]));
        double norm_lookAt = norm(c_lookAt[0],c_lookAt[1],c_lookAt[2]);
        point lookAtPoints = {c_lookAt[0]/norm_lookAt,c_lookAt[1]/norm_lookAt,c_lookAt[2]/norm_lookAt};  
        lookAt.push_back(lookAtPoints);
        //Binormal vector
        if(i==0 && j==0 && u==0){
          double d [3] = {1,1,0}; // D  
          cross_prod(lookAtPoints, d, c_sidePoints); // B0 = T0 x D
        }
        else{
          double d[3] = {upVector[upVector.size()-1].x, upVector[upVector.size()-1].y, upVector[upVector.size()-1].z};
          cross_prod(lookAtPoints, d, c_sidePoints); // B1 = T1 x N0 
        }
        double norm_sidePoints = norm(c_sidePoints[0],c_sidePoints[1],c_sidePoints[2]);
        point sidePoints = {c_sidePoints[0]/norm_sidePoints,c_sidePoints[1]/norm_sidePoints,c_sidePoints[2]/norm_sidePoints};
        sideVector.push_back(sidePoints); //add new binormal point
        //Normal vector
        cross_prod(c_sidePoints,lookAtPoints,c_upPoints); // N1 = T1 x B1
        double norm_upPoints = norm(c_upPoints[0],c_upPoints[1],c_upPoints[2]);
        point upPoints = {c_upPoints[0]/norm_upPoints,c_upPoints[1]/norm_upPoints,c_upPoints[2]/norm_upPoints};
        upVector.push_back(upPoints);
      }
    }
  }
}
void recordFrame(){
  // if(((clock() - timeSinceLastScreenshot) > 1/40 && screenShotnum<1000)){
  if(screenShotnum<1000){
    if(screenShotnum > 1){
      sprintf(myFilenm, "screenshots/%03d.jpg", screenShotnum-1); 
      saveScreenshot(myFilenm);
      // timeSinceLastScreenshot = clock();
    }
    screenShotnum += 1;
  }
  return;
}
void move(){ //move the camera through the scene
  if(currPoint < trackPoints.size()){
    int sF = 100;
    point p1 = {trackPoints[currPoint].x-sideVector[currPoint].x/sF,
                trackPoints[currPoint].y-sideVector[currPoint].y/sF,
                trackPoints[currPoint].z-sideVector[currPoint].z/sF};
    point p2 = {(lookAt[currPoint].x+trackPoints[currPoint].x), 
                (lookAt[currPoint].y+trackPoints[currPoint].y), 
                (lookAt[currPoint].z+trackPoints[currPoint].z)};
    point p3 = {upVector[currPoint].x, upVector[currPoint].y, upVector[currPoint].z};
    if(!testing){
      gluLookAt(  p1.x, p1.y, p1.z,   p2.x, p2.y, p2.z,   p3.x, p3.y, p3.z);
    }
    else{
      glColor4f(0,0,0,1);
      glBegin(GL_POLYGON);
        glVertex3f(p1.x, p1.y, p1.z); //position
        glVertex3f(p1.x+p2.x/10, p1.y+p2.y/10, p1.z+p2.z/10); //look at
        glVertex3f(p1.x+p3.x/10, p1.y+p3.y/10, p1.z+p3.z/10); //up
      glEnd();
    }
    currPoint += MOVE_INTERVAL;
    timeSinceLastMove = clock();
  }
}
void genLists(){ //define the display lists for fast drawing of skybox and track points 
  listIndex = glGenLists(10);
  lists[0]=0;lists[1]=1;lists[2]=2;lists[3]=3;lists[4]=4;lists[5]=5;lists[6]=6;lists[7]=7;lists[8]=8;lists[9]=9;
  glNewList(listIndex, GL_COMPILE);
    glEnable(GL_TEXTURE_2D); 
    int sF = 50, sF2=150,shortFactor=2,texturePointx,texturePointy; //factor by which to scale down normal/binormal/tangent vectors
    glBindTexture(GL_TEXTURE_2D, textures[7]);
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<trackPoints.size(); i+=3){ //side of rail
        if(i%2==0){texturePointx=0;texturePointy=0;}
        if(i%2==1){texturePointx=1;texturePointy=0;}
        glTexCoord2f(texturePointx, texturePointy); glVertex3f(trackPoints[i].x-upVector[i].x/sF, trackPoints[i].y-upVector[i].y/sF, trackPoints[i].z-upVector[i].z/sF);
        if(i%2==0){texturePointx=0;texturePointy=1;}
        if(i%2==1){texturePointx=1;texturePointy=1;}
        glTexCoord2f(texturePointx, texturePointy); glVertex3f(trackPoints[i].x-upVector[i].x/(sF*shortFactor), trackPoints[i].y-upVector[i].y/(sF*shortFactor), trackPoints[i].z-upVector[i].z/(sF*shortFactor));
    }
    glEnd();
  glEndList();
  glNewList(listIndex+1, GL_COMPILE);
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<trackPoints.size(); i+=3){ //side of rail
        if(i%2==0){texturePointx=0;texturePointy=0;}
        if(i%2==1){texturePointx=1;texturePointy=0;}
        glTexCoord2f(texturePointx, texturePointy); glVertex3f(trackPoints[i].x-upVector[i].x/sF-sideVector[i].x/sF, trackPoints[i].y-upVector[i].y/sF-sideVector[i].y/sF, trackPoints[i].z-upVector[i].z/sF-sideVector[i].z/sF);
        if(i%2==0){texturePointx=0;texturePointy=1;}
        if(i%2==1){texturePointx=1;texturePointy=1;}
        glTexCoord2f(texturePointx, texturePointy); glVertex3f(trackPoints[i].x-upVector[i].x/(sF*shortFactor)-sideVector[i].x/sF, trackPoints[i].y-upVector[i].y/(sF*shortFactor)-sideVector[i].y/sF, trackPoints[i].z-upVector[i].z/(sF*shortFactor)-sideVector[i].z/sF);
    }
    glEnd();
  glEndList();
  glNewList(listIndex+2, GL_COMPILE);
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<trackPoints.size()-1; i+=3){ //bottom of rail
        if(i%2==0){texturePointx=0;texturePointy=0;}
        if(i%2==1){texturePointx=1;texturePointy=0;}
        glTexCoord2f(texturePointx, texturePointy); glVertex3f(trackPoints[i].x-sideVector[i].x/sF-upVector[i].x/sF, trackPoints[i].y-sideVector[i].y/sF-upVector[i].y/sF, trackPoints[i].z-sideVector[i].z/sF-upVector[i].z/sF);
        if(i%2==0){texturePointx=0;texturePointy=1;}
        if(i%2==1){texturePointx=1;texturePointy=1;}
        glTexCoord2f(texturePointx, texturePointy); glVertex3f(trackPoints[i+1].x-upVector[i].x/sF, trackPoints[i+1].y-upVector[i].y/sF, trackPoints[i+1].z-upVector[i].z/sF);
    }
    glEnd();
  glEndList();
  glNewList(listIndex+3, GL_COMPILE);
    for(int i=0; i<trackPoints.size(); i+=TRACK_INTERVAL){ //the wooden slats that appear at every interval
      point t=lookAt[i], b_=sideVector[i], n=upVector[i], p=trackPoints[i];
      int hs = sF*shortFactor, fs=sF*shortFactor*9;
      point a = {p.x-n.x/sF2-n.x/hs,p.y-n.y/sF2-n.y/hs,p.z-n.z/sF2-n.z/hs};
      point b = {p.x-b_.x/sF-n.x/sF2-n.x/hs,p.y-b_.y/sF-n.y/sF2-n.y/hs,p.z-b_.z/sF-n.z/sF2-n.z/hs};
      point c = {p.x-b_.x/sF-n.x/sF, p.y-b_.y/sF-n.y/sF, p.z-b_.z/sF-n.z/sF };
      point d = {p.x-n.x/sF,p.y-n.y/sF,p.z-n.z/sF};
      
      point e = {p.x+t.x/fs-n.x/sF2-n.x/hs,p.y+t.y/fs-n.y/sF2-n.y/hs,p.z+t.z/fs-n.z/sF2-n.z/hs};
      point f = {p.x+t.x/fs-b_.x/sF-n.x/sF2-n.x/hs,p.y+t.y/fs-b_.y/sF-n.y/sF2-n.y/hs,p.z+t.z/fs-b_.z/sF-n.z/sF2-n.z/hs};
      point g = {p.x+t.x/fs-b_.x/sF-n.x/sF, p.y+t.y/fs-b_.y/sF-n.y/sF, p.z+t.z/fs-b_.z/sF-n.z/sF};
      point h = {p.x+t.x/fs-n.x/sF,p.y+t.y/fs-n.y/sF,p.z+t.z/fs-n.z/sF};
          glBindTexture(GL_TEXTURE_2D, textures[6]); 
          glBegin(GL_POLYGON); //back
            glTexCoord2f(0.0f,0.0f); glVertex3f(a.x, a.y, a.z);
            glTexCoord2f(0.0f,1.0f); glVertex3f(b.x, b.y, b.z);
            glTexCoord2f(0.0f,0.0f); glVertex3f(c.x, c.y, c.z);
            glTexCoord2f(1.0f,1.0f); glVertex3f(d.x, d.y, d.z);
          glEnd();
          glBegin(GL_POLYGON); //front
            glTexCoord2f(0.0f,0.0f); glVertex3f(e.x, e.y, e.z);
            glTexCoord2f(0.0f,1.0f); glVertex3f(f.x, f.y, f.z);
            glTexCoord2f(1.0f,1.0f); glVertex3f(g.x, g.y, g.z);
            glTexCoord2f(0.0f,0.0f); glVertex3f(h.x, h.y, h.z);
          glEnd();
          glBegin(GL_POLYGON); //top
            glTexCoord2f(0.0f,0.0f); glVertex3f(a.x, a.y, a.z);
            glTexCoord2f(0.0f,1.0f); glVertex3f(e.x, e.y, e.z);
            glTexCoord2f(1.0f,1.0f); glVertex3f(f.x, f.y, f.z);
            glTexCoord2f(0.0f,0.0f); glVertex3f(b.x, b.y, b.z);
          glEnd();
          glBegin(GL_POLYGON); //bottom
            glTexCoord2f(0.0f,0.0f); glVertex3f(c.x, c.y, c.z);
            glTexCoord2f(0.0f,1.0f); glVertex3f(g.x, g.y, g.z);
            glTexCoord2f(1.0f,1.0f); glVertex3f(f.x, f.y, f.z);
            glTexCoord2f(0.0f,0.0f); glVertex3f(d.x, d.y, d.z);
          glEnd();
    }
  glEndList();
  //skybox list
  glNewList(listIndex+4, GL_COMPILE);
    int mF = 3000;
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, textures[0]); //front
    glBegin(GL_POLYGON); 
      glTexCoord2f(1.0f,1.0f); glVertex3f(-1*mF,-1*mF,-1*mF);
      glTexCoord2f(0.0f,1.0f); glVertex3f( 1*mF,-1*mF,-1*mF);
      glTexCoord2f(0.0f,0.0f); glVertex3f( 1*mF, 1*mF,-1*mF);
      glTexCoord2f(1.0f,0.0f); glVertex3f(-1*mF, 1*mF,-1*mF);
    glEnd();  
  glEndList();
  glNewList(listIndex+5, GL_COMPILE);
    glBindTexture(GL_TEXTURE_2D, textures[1]); //right
    glBegin(GL_POLYGON); 
      glTexCoord2f(1.0f,1.0f); glVertex3f(1*mF,-1*mF,-1*mF);
      glTexCoord2f(0.0f,1.0f); glVertex3f(1*mF,-1*mF, 1*mF);
      glTexCoord2f(0.0f,0.0f); glVertex3f(1*mF, 1*mF, 1*mF);
      glTexCoord2f(1.0f,0.0f); glVertex3f(1*mF, 1*mF,-1*mF);
    glEnd();  
  glEndList();
  glNewList(listIndex+6, GL_COMPILE);
    glBindTexture(GL_TEXTURE_2D, textures[2]); //left
    glBegin(GL_POLYGON); 
      glTexCoord2f(0.0f,1.0f); glVertex3f(-1*mF,-1*mF,-1*mF);
      glTexCoord2f(1.0f,1.0f); glVertex3f(-1*mF,-1*mF, 1*mF);
      glTexCoord2f(1.0f,0.0f); glVertex3f(-1*mF, 1*mF, 1*mF);
      glTexCoord2f(0.0f,0.0f); glVertex3f(-1*mF, 1*mF,-1*mF);
    glEnd();  
  glEndList();
  glNewList(listIndex+7, GL_COMPILE);
    glBindTexture(GL_TEXTURE_2D, textures[3]); //top
    glBegin(GL_POLYGON); 
      glTexCoord2f(0.0f,0.0f); glVertex3f( 1*mF, 1*mF, 1*mF);
      glTexCoord2f(1.0f,0.0f); glVertex3f(-1*mF, 1*mF, 1*mF);
      glTexCoord2f(1.0f,1.0f); glVertex3f(-1*mF, 1*mF,-1*mF);
      glTexCoord2f(0.0f,1.0f); glVertex3f( 1*mF, 1*mF,-1*mF);
    glEnd();  
  glEndList();
  glNewList(listIndex+8, GL_COMPILE);
    glBindTexture(GL_TEXTURE_2D, textures[4]); //back
    glBegin(GL_POLYGON); 
      glTexCoord2f(0.0f,1.0f); glVertex3f(-1*mF,-1*mF, 1*mF);
      glTexCoord2f(1.0f,1.0f); glVertex3f( 1*mF,-1*mF, 1*mF);
      glTexCoord2f(1.0f,0.0f); glVertex3f( 1*mF, 1*mF, 1*mF);
      glTexCoord2f(0.0f,0.0f); glVertex3f(-1*mF, 1*mF, 1*mF);
    glEnd();
  glEndList();
  glNewList(listIndex+9, GL_COMPILE);
    glBindTexture(GL_TEXTURE_2D, textures[5]); //bottom
    glBegin(GL_POLYGON); 
      glTexCoord2f(0.0f,1.0f); glVertex3f( 1*mF,-1*mF, 1*mF);
      glTexCoord2f(1.0f,1.0f); glVertex3f(-1*mF,-1*mF, 1*mF);
      glTexCoord2f(1.0f,0.0f); glVertex3f(-1*mF,-1*mF,-1*mF);
      glTexCoord2f(0.0f,0.0f); glVertex3f( 1*mF,-1*mF,-1*mF);
    glEnd();
    glDisable(GL_TEXTURE_2D);
  glEndList();
}
void renderFloor(){ //use the heightmap from assignment 1 (currently unused)
  glRotatef(90,1,0,0);
  glEnable(GL_TEXTURE_2D); 
  // glBindTexture(GL_TEXTURE_2D, textures[8]); 
  int sF = 100;
  for(int i=0; i<g_pHeightData->nx-1; i++){
    glBegin(GL_TRIANGLE_STRIP);
      for(int j=0; j<g_pHeightData->ny; j++){
        //calculate the height of the bottom and top vertices
        float height_1 = 10*(float)PIC_PIXEL(g_pHeightData, i, j, 0) / max_value;
        float height_2 = 10*(float)PIC_PIXEL(g_pHeightData, i+1, j, 0) / max_value;
        // glColor4f(2*height_1, 2*height_1, 1, 1); 
        if(j%2) glTexCoord2f(1,0);
        else glTexCoord2f(0,0);
        glVertex3f( -sF*(((float)i/(float)g_pHeightData->nx)-.5),
                    sF*(((float)j/(float)g_pHeightData->ny)-.5),
                    height_1 );
        // glColor4f(2*height_2, 2*height_2, 1, 1);
        if(j%2) glTexCoord2f(1,1);
        else glTexCoord2f(0,1);
        glVertex3f( -sF*(((float)(i+1)/(float)g_pHeightData->nx)-.5),
                    sF*(((float)j/(float)g_pHeightData->ny)-.5),
                    height_2 );
      }
    glEnd();
  }
}
void display(){ 
  //control frames per second
  if(((float)(clock()- timeSinceLastMove)/CLOCKS_PER_SEC) >= (1.0/FRAMES_PER_SECOND)){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity(); // reset transformation
    if(!testing) move(); //move the camera
    if(testing){
      // transformations through modifiers shift, ctrl/alt, and default rotate 
      glTranslatef(g_vLandTranslate[0],g_vLandTranslate[1],-15+g_vLandTranslate[2]);
      glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);
      glRotatef(sqrt(pow(g_vLandRotate[0],2)+pow(g_vLandRotate[1],2)), g_vLandRotate[0], g_vLandRotate[1], g_vLandRotate[2]);
      move(); //move the test triangle
    }
    //call the ten display lists to draw track and skybox efficiently
    glListBase(listIndex); 
    glCallLists(10, GL_UNSIGNED_BYTE, lists);
    // if(renderHeightmap) renderFloor(); //render a floor using heightmap (didn't look as good without lighting so I took it out)
    if(record) //record frame
      recordFrame(); 
    glutSwapBuffers(); // double buffer flush
    timeSinceLastMove = clock();
  }
}
void doIdle(){
  if(currPoint < trackPoints.size()) glutPostRedisplay();
}
// called every time window is resized to update projection matrix
void reshape(int w, int h){
  glMatrixMode(GL_MODELVIEW);
  // setup image size
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // GLfloat aspect = (GLfloat) w / (GLfloat) h;
  // setup camera
  // glFrustum(-0.1, 0.1, -float(h)/(10.0*float(w)), float(h)/(10.0*float(w)), 0.5, 1000.0);
  // glFrustum(-0.1, 0.1, -float(h)/(10.0*float(w)), float(h)/(10.0*float(w)), 0.1, 5000.0);
  if(!testing)
    // glOrtho(-2.0, 2.0, -2.0/aspect, 2.0/aspect, -10.0, 10.0); 
    gluPerspective(100, (float)(w) / h, 0.001f, 9000.0f);
  else
    glFrustum(-0.1, 0.1, -float(h)/(10.0*float(w)), float(h)/(10.0*float(w)), 0.1, 5000.0);

  // glOrtho(-.1, .1, -.1, .1, .1, 5000);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  window_width = glutGet(GLUT_WINDOW_WIDTH);
  window_height = glutGet(GLUT_WINDOW_HEIGHT);
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
void menufunc(int value){
  switch (value){
    case 0: //restart
      currPoint = 0;
      break;
    case 1: //quit
      exit(0);
  }
}
int main (int argc, char ** argv){
  if (argc<2){  
    printf ("usage: %s <trackfile>\n", argv[0]);
    exit(0);
  }
  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA); // request double buffer
  glutInitWindowSize(640,480); // set window size
  glutInitWindowPosition(0, 0); // set window position
  glutCreateWindow("Roller Coaster"); // creates a window
  /* allow the user to quit or restart using the right mouse button menu */
  g_iMenuId = glutCreateMenu(menufunc);
  glutSetMenu(g_iMenuId);
  glutAddMenuEntry("Restart",0);
  glutAddMenuEntry("Quit",1);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  glutIdleFunc(doIdle); //animation
  if(testing){
    glutMotionFunc(mousedrag); //mouse drags
    glutPassiveMotionFunc(mouseidle); //idle mouse
    glutMouseFunc(mousebutton); //mouse buttons
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
  }

  // if(renderHeightmap){
  //   char* heightmap_filepath = convertStringToChar("Assets/OhioPyle-768.jpg");
  //   g_pHeightData = jpeg_read(heightmap_filepath, NULL);
  //   if (!g_pHeightData){
  //     printf ("error reading %s.\n", heightmap_filepath);
  //     exit(1);
  //   }
  // }
  myinit();
  glutReshapeFunc(reshape); //reshape func
  glutDisplayFunc(display); //display func
  loadSplines(argv[1]);
  calculateSplines();
  initTextures();
  genLists();
  // timeSinceLastScreenshot = clock();
  timeSinceLastMove = clock();
  glutMainLoop(); //start program gui

  return 0;
}
//ffmpeg -f image2 -r 15 -i %03d.jpg -vcodec mpeg4 -y movie.mp4
