Assignment #3: Ray tracing

FULL NAME: Ahsan Zaman


MANDATORY FEATURES
------------------

<Under "Status" please indicate whether it has been implemented and is
functioning correctly.  If not, please explain the current status.>

Feature:                                 Status: finish? (yes/no)
-------------------------------------    -------------------------
1) Ray tracing triangles                  YES

2) Ray tracing sphere                     YES

3) Triangle Phong Shading                 YES

4) Sphere Phong Shading                   YES

5) Shadows rays                           YES

6) Still images                           YES
   
7) Extra Credit (up to 20 points)
	1. I implemented recursive ray tracing. Reflections can be viewed if you enable recursion by setting the global variable performRecursiveRayTrace to TRUE on line 39 of assign3.cpp.

	2. I also implemented an anti-aliasing function using the supersampling method. You can enable anti-aliasing by setting the performAntiAlias global variable to TRUE on line 38 of assign3.cpp.

	NOTE: Both recursion and anti-aliasing are ON by default. The still images show the difference between enabling these extra credit features and disabling them. 


8) Instructions for running the program:
	Simply type make in the directory with the Makefile to build the program. 
	Run the program with this command:
	./assign3 SceneFiles/<scenefile> output.jpeg
	where scenefile is the name of a scene in the SceneFiles folder.
	Make sure the Pic library has been built and exists at the same directory level as this project.
