/*
CSCI 420
Assignment 3 Raytracer

Name: Ahsan Zaman
Date: December 8, 2018
Email: ahsanzam@usc.edu
*/

#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>
#include <string.h>
#include <math.h>
#include <iostream>

#define MAX_TRIANGLES 2000
#define MAX_SPHERES 10
#define MAX_LIGHTS 10
using namespace std;
char *filename=0;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2
int mode=MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480

//the field of view of the camera
#define fov 60.0

unsigned char buffer[HEIGHT][WIDTH][3];
bool performAntiAlias = true;
bool performRecursiveRayTrace = true;

struct Vertex{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double normal[3];
  double shininess;
};

typedef struct _Triangle{
  struct Vertex v[3];
} Triangle;

typedef struct _Sphere{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double shininess;
  double radius;
} Sphere;

typedef struct _Light{
  double position[3];
  double color[3];
} Light;

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles = 0;
int num_spheres   = 0;
int num_lights    = 0;

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);
double clamp(double val, double min, double max){ //clamps a value between min and max
  return ( val < min ) ? min : ( val > max ) ? max : val; //this multi-layered conditional syntax is pretty cool!
}
// Struct for 3D vectors; allows for easily accessing commonly used vector functions  
struct Vector {
  double x,y,z;
  Vector ( ): x(0), y(0), z(0) {}
  Vector (double* d): x(d[0]), y(d[1]), z(d[2]) { }
  Vector (double x,double y,double z): x(x), y(y), z(z) { } 
  Vector operator+ (double const& s) const { return Vector(this->x+s, this->y+s, this->z+s);                }
  Vector operator+ (Vector const& v) const { return Vector(this->x+v.x,this->y+v.y,this->z+v.z);            }
  Vector operator* (double const& s) const { return Vector(this->x*s, this->y*s, this->z*s);                }
  Vector operator* (Vector const& v) const { return Vector(this->x*v.x, this->y*v.y, this->z*v.z);          }
  Vector operator- (double const& s) const { return Vector(this->x-s, this->y-s, this->z-s);                }
  Vector operator- (Vector const& v) const { return Vector(this->x-v.x, this->y-v.y, this->z-v.z);          }
  Vector operator/ (double const& s) const { return Vector(this->x/s, this->y/s, this->z/s);                }
  bool  operator== (Vector const& v) const { return ((this->x == v.x && this->y == v.y) && this->z == v.z); }
  double dot       (Vector v)        const { return (this->x * v.x) + (this->y * v.y) + (this->z * v.z);    }
  Vector normalize ( )               const {return *this/sqrt(pow(this->x,2)+pow(this->y,2)+pow(this->z,2));}
  bool isZero      ( )               const { return ((this->x==0 && this->y==0) && this->z==0);             }
  double length    ( )               const { return sqrt(pow(this->x,2)+pow(this->y,2)+pow(this->z,2));     }
  void print       ( )               const { cout << "Vec x: " << this->x << 
  														" y: " << this->y << " z: " << this->z << endl; 	}
  Vector cross     (Vector v)        const { //cross product
    return Vector(  (this->y * v.z) - (this->z * v.y),
                    (this->z * v.x) - (this->x * v.z),
                    (this->x * v.y) - (this->y * v.x) ); 													}
};
// this color struct has basically the same member vars as a vector struct
//  with the addition of clamping inputs by 256
//  can't inherit from Vector because need to call vars by r,g,b instead of x,y,z
//  also need to clamp color values when they go out of bounds during color addition
struct Color { 
  double r,g,b;
  Color ( ) : r(0), g(0), b(0) { }
  Color (double* d) : r(d[0]), g(d[1]), b(d[2]) { }
  Color (double r,double g,double b) : r(r), g(g), b(b) { }
  Color operator* (Color   const& c)   const { return Color( (this->r*c.r), (this->g*c.g), (this->b*c.b) );   } 
  Color operator* (double  const& s)   const { return Color( (this->r*s), (this->g*s), (this->b*s) );         } 
  Color operator/ (double  const& s)   const { return Color( (this->r/s), (this->g/s), (this->b/s) );         } 
  Color operator+ (Color   const& c)   const { return Color( (this->r+c.r), (this->g+c.g), (this->b+c.b) );   } 
  Color operator+ (double  const& s)   const { return Color( (this->r+s), (this->g+s), (this->b+s) );         } 
  bool  operator!= (double  const& s)  const { return ((this->r != s && this->g != s) && this->b != s);       } 
  bool  operator== (double  const& s)  const { return ((this->r == s && this->g == s) && this->b == s);       } 
  bool  operator>= (double  const& s)  const { return ((this->r >= s && this->g >= s) && this->b >= s);       } 
  bool  operator>  (double  const& s)  const { return ((this->r > s && this->g > s) && this->b > s);          } 
  bool  operator<  (double  const& s)  const { return ((this->r < s && this->g < s) && this->b < s);          } 
  bool  operator<= (double  const& s)  const { return ((this->r <= s && this->g <= s) && this->b <= s);       } 
  bool  operator!= (Color   const& c)  const { return ((this->r != c.r && this->g != c.g) && this->b != c.b); } 
  bool  operator== (Color   const& c)  const { return ((this->r == c.r && this->g == c.g) && this->b == c.b); } 
  Color deNormalize( )                 const { return Color((this->r*255.0),(this->g*255.0),(this->b*255.0)); }
  Color clamp(double min, double max)  const { return Color( ::clamp(this->r,min,max), 
  													  ::clamp(this->g,min,max), ::clamp(this->b, min, max) ); }
  void  print      ( )                 const { cout << "Color R: " << this->r << 
  															" G: " << this->g << " B: " << this->b << endl;   }  
  Color normalize  ( )                 const { 
    double length = sqrt(pow(this->r,2)+pow(this->g,2)+pow(this->b,2));
    return Color(this->r / length, this->g / length, this->b / length); 									  }
};
struct Ray{
  Vector direction, origin;
  Ray() { }
  Ray(Vector o, Vector d) : direction(d.normalize()), origin(o) { }
};
Ray rayGen(double screen_x, double screen_y, double offset_x, double offset_y){
  double aspect_ratio = (double)WIDTH / (double)HEIGHT;
  // using formula given in slides multiplied with the given position normalized after a slight offset
  double world_x = aspect_ratio * tan( (fov / 2.0) * (M_PI / 180.0) ) * ( 2 * (screen_x + offset_x) / WIDTH - 1 );
  double world_y = tan( (fov / 2.0) * (M_PI / 180.0) ) * ( 2 * (screen_y + offset_y) / HEIGHT - 1 ); 
  double world_z = -1 ; 
  Vector origin = Vector(0,0,0);
  Vector direction = Vector(world_x, world_y, world_z).normalize(); 
  return Ray(origin, direction);
}
Ray rayGen(double x,double y){ return rayGen(x,y,0,0); } // rayGen wrapper for when there is no offset

//decides whether vector a is closer to the origin
bool vectorAIsCloser(Vector a, Vector b, Vector origin){
  double a_dist = fabs((a - origin).length());
  double b_dist = fabs((b - origin).length());
  return a_dist < b_dist;
}

//checks given ray for collision with given sphere. 
//  If there is a collision, this saves the position of the collision in prevCollision vector
bool checkSphereCollisions(Ray ray, Sphere sphere, Vector* prevCollision){
  //following equation is given in slides: 
  //  sphere is given by center [x_c, y_c, z_c] and radius r
  //  ray is given by vectors origin and direction
  //  get implicit surface for sphere: 0 = (x - x_c)^2 + (y - y_c)^2 + (z - z_c)^2 - r^2
  //  plug ray equations for x, y, z into implicit surface equation
  //   -> 0 = (x_0 + x_d * t - x_c)^2 + (y_0 + y_d * t - y_c)^2 + (z_0 + z_d * t - z_c)^2 - r^2
  //  then solve the resulting quadratic equation 
  double a = 1; //since direction is normalized
  double b = 2 * (ray.direction.x * (ray.origin.x - sphere.position[0]) + ray.direction.y * (ray.origin.y - sphere.position[1]) + ray.direction.z * (ray.origin.z - sphere.position[2]));
  double c = pow(ray.origin.x - sphere.position[0],2) + pow(ray.origin.y - sphere.position[1],2) + pow(ray.origin.z - sphere.position[2],2) - pow(sphere.radius, 2);
  double solution_1 = (-1.0 * b + sqrt(pow(b,2) - 4.0 * a * c)) / 2.0; //there was a tough-to-find error was here: missing parentheses resulting in wrong order of operations. sphere would be in shadow  
  double solution_2 = (-1.0 * b - sqrt(pow(b,2) - 4.0 * a * c)) / 2.0;
  double minimum = fmin(solution_1, solution_2);
  if(minimum > 0){ 
    Vector currCollision = ray.origin + (ray.direction * minimum);
    if(vectorAIsCloser(currCollision, *prevCollision, ray.origin)|| *prevCollision == ray.origin){
      *prevCollision = currCollision;
      return true;
    }
  }
  return false;
}

// Check for an collision between a given ray and a triangle
//  The Möller-Trumbore algorithm used here is a fast triangle-ray collision detection method referenced from
//    https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
bool checkTriangleCollisions(Ray ray, Triangle triangle, Vector* prevCollision, Vector* bCoords){ 
  Vector triangle_point_0 = Vector(triangle.v[0].position[0], triangle.v[0].position[1], triangle.v[0].position[2]);
  Vector triangle_point_1 = Vector(triangle.v[1].position[0], triangle.v[1].position[1], triangle.v[1].position[2]);
  Vector triangle_point_2 = Vector(triangle.v[2].position[0], triangle.v[2].position[1], triangle.v[2].position[2]);
  Vector v1to0 = triangle_point_1 - triangle_point_0;
  Vector v2to0 = triangle_point_2 - triangle_point_0;
  Vector pvec = ray.direction.cross(v2to0);
  double determinant = v1to0.dot(pvec);
  // if(determinant < 0) return false; //a tough to find error was here. 

  double inverseDeterminant = 1 / determinant;
  Vector tvec = ray.origin - triangle_point_0;
  double u = tvec.dot(pvec) * inverseDeterminant;
  if (u < 0 || u > 1) return false; //check if ray is parallel to triangle

  Vector qvec = tvec.cross(v1to0);
  double v = ray.direction.dot(qvec) * inverseDeterminant;
  if(v < 0 || u + v > 1) return false;  //check if ray intersects triangle

  //at this point, we know there is an intersection. 
  // now we have to check if this intersection is the closest
  double t = v2to0.dot(qvec) * inverseDeterminant; //calculate distance from origin at which collision occurs
  if(t <= 0) return false; //checks if triangle is behind ray origin
  Vector collisionPoint = ray.origin + ray.direction * t; //calculate collision point using ray equation
  if(vectorAIsCloser(collisionPoint, *prevCollision, ray.origin) || *prevCollision == ray.origin){ //compare this collision with closest previous collision
    *prevCollision = collisionPoint; //update collision point
    *bCoords = Vector(u, v, 1.0 - u - v); // calculate baryocentric coordinates
    return true;
  }
  return false;
}

bool checkAllTrianglesForCollisions(Ray ray, int* triangleCollisionIndex, Vector* collisionPoint, Vector* bCoords){
  bool triangleCollisionOccurred = false; 
  for(int i=0; i<num_triangles; i++){
    if(checkTriangleCollisions(ray, triangles[i], collisionPoint, bCoords)){
      triangleCollisionOccurred = true;
      *triangleCollisionIndex = i;      
    }
  }
  return triangleCollisionOccurred;
}
bool checkAllCirclesForCollisions(Ray ray, int* sphereCollisionIndex, Vector* collisionPoint){
  bool sphereCollisionOccurred = false;
  for(int i=0; i<num_spheres; i++){
    if(checkSphereCollisions(ray, spheres[i], collisionPoint)){
      sphereCollisionOccurred = true;
      *sphereCollisionIndex = i;
    }
  }
  return sphereCollisionOccurred;
}

// checks for intersections between given ray and objects in the scene 
Color fireRay(Ray ray, int iterationNum){
  Vector bCoords; Vector collisionPoint(1000,1000,1000);
  int triangleCollisionIndex=-1; int sphereCollisionIndex=-1;
  Color illumination(0,0,0);

  //check for collisions between origin ray through screen and objects in the scene
  // first check for triangle collisions, then check for sphere collisions with ray
  bool triangleCollisionOccurred = checkAllTrianglesForCollisions(ray, &triangleCollisionIndex, &collisionPoint, &bCoords);
  bool sphereCollisionOccurred   = checkAllCirclesForCollisions(ray, &sphereCollisionIndex, &collisionPoint);

  //if there is no object that collides with this ray, we can just return the white background color
  if((!sphereCollisionOccurred && !triangleCollisionOccurred) || iterationNum >= 3)
    return Color(1.0, 1.0, 1.0);

  Color diffuseIllumination, specularIllumination; //store diffuse and specular properties of surface here
  Vector normal; //store normal vector of collision point here
  double shininess; //store shininess of surface here
  if(sphereCollisionOccurred){
    Vector sphereCenter(spheres[sphereCollisionIndex].position);
    normal = (collisionPoint - sphereCenter).normalize(); //something might be wrong here
    diffuseIllumination  = Color(spheres[sphereCollisionIndex].color_diffuse);
    specularIllumination = Color(spheres[sphereCollisionIndex].color_specular);
    shininess = spheres[sphereCollisionIndex].shininess;
  }
  else { // if sphere collision did not occur and we are here, then triangle collision occurred
    //interpolate using the baryocentric coords
    //  interpolate normals 
    normal = Vector(  triangles[triangleCollisionIndex].v[0].normal[0]*bCoords.z+triangles[triangleCollisionIndex].v[1].normal[0]*bCoords.x+triangles[triangleCollisionIndex].v[2].normal[0]*bCoords.y,
                      triangles[triangleCollisionIndex].v[0].normal[1]*bCoords.z+triangles[triangleCollisionIndex].v[1].normal[1]*bCoords.x+triangles[triangleCollisionIndex].v[2].normal[1]*bCoords.y,
                      triangles[triangleCollisionIndex].v[0].normal[2]*bCoords.z+triangles[triangleCollisionIndex].v[1].normal[2]*bCoords.x+triangles[triangleCollisionIndex].v[2].normal[2]*bCoords.y ).normalize();
    //  interpolate specular component
    specularIllumination  = Color(  triangles[triangleCollisionIndex].v[0].color_specular[0]*bCoords.z+triangles[triangleCollisionIndex].v[1].color_specular[0]*bCoords.x+triangles[triangleCollisionIndex].v[2].color_specular[0]*bCoords.y,
                                    triangles[triangleCollisionIndex].v[0].color_specular[1]*bCoords.z+triangles[triangleCollisionIndex].v[1].color_specular[1]*bCoords.x+triangles[triangleCollisionIndex].v[2].color_specular[1]*bCoords.y,
                                    triangles[triangleCollisionIndex].v[0].color_specular[2]*bCoords.z+triangles[triangleCollisionIndex].v[1].color_specular[2]*bCoords.x+triangles[triangleCollisionIndex].v[2].color_specular[2]*bCoords.y );
    //  interpolate diffuse component
    diffuseIllumination = Color(  triangles[triangleCollisionIndex].v[0].color_diffuse[0]*bCoords.z+triangles[triangleCollisionIndex].v[1].color_diffuse[0]*bCoords.x+triangles[triangleCollisionIndex].v[2].color_diffuse[0]*bCoords.y,
                                  triangles[triangleCollisionIndex].v[0].color_diffuse[1]*bCoords.z+triangles[triangleCollisionIndex].v[1].color_diffuse[1]*bCoords.x+triangles[triangleCollisionIndex].v[2].color_diffuse[1]*bCoords.y,
                                  triangles[triangleCollisionIndex].v[0].color_diffuse[2]*bCoords.z+triangles[triangleCollisionIndex].v[1].color_diffuse[2]*bCoords.x+triangles[triangleCollisionIndex].v[2].color_diffuse[2]*bCoords.y );
    //  interpolate shininess using baryocentric coordinates
    shininess = triangles[triangleCollisionIndex].v[0].shininess*bCoords.z + triangles[triangleCollisionIndex].v[1].shininess*bCoords.x + triangles[triangleCollisionIndex].v[0].shininess*bCoords.y;
  }
  //calculate illumination using the phong shading formula for each light in scene that is not blocked by another object 
  Vector collisionToCamera = (ray.origin - collisionPoint).normalize(); //vector to camera
  for(int i=0; i<num_lights; i++){
    // generate shadow ray
    Ray shadowRay = Ray(Vector(collisionPoint + (normal * .001)), (Vector(lights[i].position) - collisionPoint).normalize());
    // decide if the path from the collision to this light is blocked by an object  
    int shadowTriangleCollisionIndex=-1, shadowSphereCollisionIndex=-1;
    Vector shadowCollisionPoint(lights[i].position); Vector shadowBCoords;
    bool shadowRayTriangleCollisionOccurred = checkAllTrianglesForCollisions(shadowRay, &shadowTriangleCollisionIndex, &shadowCollisionPoint, &shadowBCoords);
    bool shadowRaySphereCollisionOccurred   = checkAllCirclesForCollisions(shadowRay, &shadowSphereCollisionIndex, &shadowCollisionPoint);
    if(!shadowRayTriangleCollisionOccurred && !shadowRaySphereCollisionOccurred){
      // Save diffuse, specular, and shiny components to calculate phong shading if path to this light source not occluded by another object
      // Phong shading equation: I = lightColor * (k_d * (L dot N) + k_s * (R dot V) ^ sh)
      Vector l = (Vector(lights[i].position) - collisionPoint).normalize(); //vector to light 
      Vector r = ((normal * 2 * l.dot(normal)) - l).normalize(); // reflection ray about n
      Color diffuse_component = diffuseIllumination * clamp(l.dot(normal),0,1);  
      Color specular_component = specularIllumination * pow(clamp((collisionToCamera.dot(r)),0,1),shininess);
      illumination = (illumination + (Color(lights[i].color) * ( diffuse_component + specular_component ))).clamp(0, 1);
    }
  }
  //perform recursive ray tracing for reflections
  if(performRecursiveRayTrace){
    Vector reflectionDirection = ((normal * 2 * collisionToCamera.dot(normal)) - collisionToCamera).normalize();
    Ray reflectionRay(collisionPoint + (reflectionDirection * .01), reflectionDirection);
    Color reflectionContribution = fireRay(reflectionRay, iterationNum + 1);
    // Reflective illumination equation: (1 - k_s) * localPhongColor + k_s * colorOfReflectedRay
    illumination = (((specularIllumination * -1 ) + 1) * illumination ) + ( specularIllumination * reflectionContribution );
  }
  if(iterationNum == 0) //only add the ambient color contribution once
    illumination = illumination + Color(ambient_light);
  return illumination;
}
void draw_scene(){
  unsigned int x,y;
  glPointSize(2.0);
  #pragma omp parallel for shared(x, y)
  for(x=0; x<WIDTH; x++){
    #pragma omp for 
    for(y=0;y < HEIGHT;y++){
      glBegin(GL_POINTS);
      Color color;
      if(!performAntiAlias) // generate a ray from camera through point (x,y,-1)
        color = ((fireRay(rayGen(x,y), 0)).clamp(0,1)).deNormalize();
      else{ //anti alias mode
          int num_rays = 2;
          for(int i=0; i<num_rays; i++){
            for(int j=0; j<num_rays; j++){
              Color newColor = fireRay(rayGen(x,  y, i/num_rays, j/num_rays), 0).clamp(0,1).deNormalize();
              color = color + newColor;
            }
          }
          color = color / pow(num_rays,2);
      }
      //Combine color of any objects that intersected the ray with ambient light's color contribution
      //display calculated color as a pixel
      plot_pixel(x, y, color.r, color.g, color.b);
      glEnd();
    }
    glFlush(); 
  }
  printf("Done!\n"); fflush(stdout);
}

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b){
  glColor3f(((double)r)/256.f,((double)g)/256.f,((double)b)/256.f);
  glVertex2i(x,y);
}

void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b){
  buffer[HEIGHT-y-1][x][0]=r;
  buffer[HEIGHT-y-1][x][1]=g;
  buffer[HEIGHT-y-1][x][2]=b;
}

void plot_pixel(int x,int y,unsigned char r,unsigned char g, unsigned char b){
  plot_pixel_display(x,y,r,g,b);
  if(mode == MODE_JPEG)
      plot_pixel_jpeg(x,y,r,g,b);
}

void save_jpg(){
  Pic *in = NULL;

  in = pic_alloc(640, 480, 3, NULL);
  printf("Saving JPEG file: %s\n", filename);

  memcpy(in->pix,buffer,3*WIDTH*HEIGHT);
  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);      
}

void parse_check(char *expected,char *found){
  if(strcasecmp(expected,found))
    {
      char error[100];
      printf("Expected '%s ' found '%s '\n",expected,found);
      printf("Parse error, abnormal abortion\n");
      exit(0);
    }
}

void parse_doubles(FILE*file, char *check, double p[3]){
  char str[100];
  fscanf(file,"%s",str);
  parse_check(check,str);
  fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
  printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE*file,double *r){
  char str[100];
  fscanf(file,"%s",str);
  parse_check("rad:",str);
  fscanf(file,"%lf",r);
  printf("rad: %f\n",*r);
}

void parse_shi(FILE*file,double *shi){
  char s[100];
  fscanf(file,"%s",s);
  parse_check("shi:",s);
  fscanf(file,"%lf",shi);
  printf("shi: %f\n",*shi);
}

int loadScene(char *argv){
  FILE *file = fopen(argv,"r");
  if(file == NULL) {
    cout << "ERROR OPENING SCREENFILE" << endl;
    exit(1);
  }
  int number_of_objects;
  char type[50];
  int i;
  Triangle t;
  Sphere s;
  Light l;
  fscanf(file,"%i",&number_of_objects);

  printf("number of objects: %i\n",number_of_objects);
  char str[200];

  parse_doubles(file,"amb:",ambient_light);

  for(i=0;i < number_of_objects;i++)
    {
      fscanf(file,"%s\n",type);
      printf("%s\n",type);
      if(strcasecmp(type,"triangle")==0)
  {

    printf("found triangle\n");
    int j;

    for(j=0;j < 3;j++)
      {
        parse_doubles(file,"pos:",t.v[j].position);
        parse_doubles(file,"nor:",t.v[j].normal);
        parse_doubles(file,"dif:",t.v[j].color_diffuse);
        parse_doubles(file,"spe:",t.v[j].color_specular);
        parse_shi(file,&t.v[j].shininess);
      }

    if(num_triangles == MAX_TRIANGLES)
      {
        printf("too many triangles, you should increase MAX_TRIANGLES!\n");
        exit(0);
      }
    triangles[num_triangles++] = t;
  }
      else if(strcasecmp(type,"sphere")==0)
  {
    printf("found sphere\n");

    parse_doubles(file,"pos:",s.position);
    parse_rad(file,&s.radius);
    parse_doubles(file,"dif:",s.color_diffuse);
    parse_doubles(file,"spe:",s.color_specular);
    parse_shi(file,&s.shininess);

    if(num_spheres == MAX_SPHERES)
      {
        printf("too many spheres, you should increase MAX_SPHERES!\n");
        exit(0);
      }
    spheres[num_spheres++] = s;
  }
      else if(strcasecmp(type,"light")==0)
  {
    printf("found light\n");
    parse_doubles(file,"pos:",l.position);
    parse_doubles(file,"col:",l.color);

    if(num_lights == MAX_LIGHTS)
      {
        printf("too many lights, you should increase MAX_LIGHTS!\n");
        exit(0);
      }
    lights[num_lights++] = l;
  }
      else
  {
    printf("unknown type in scene description:\n%s\n",type);
    exit(0);
  }
    }
  return 0;
}

void display(){ }

void init(){
  glMatrixMode(GL_PROJECTION);
  glOrtho(0,WIDTH,0,HEIGHT,1,-1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void idle(){
  //hack to make it only draw once
  static int once=0;
  if(!once){
      draw_scene();
      if(mode == MODE_JPEG) 
        save_jpg();
  }
  once=1;
}

int main (int argc, char ** argv){
  if (argc<2 || argc > 3){  
    printf ("usage: %s <scenefile> [jpegname]\n", argv[0]);
    exit(0);
  }
  if(argc == 3){
    mode = MODE_JPEG;
    filename = argv[2];
  }
  else if(argc == 2) 
    mode = MODE_DISPLAY;

  glutInit(&argc,argv);
  loadScene(argv[1]);

  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(WIDTH,HEIGHT);
  int window = glutCreateWindow("Ray Tracer");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  init();
  glutMainLoop();
}
