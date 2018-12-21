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

	3. I also implemented parallelization through multi-threading using OpenMP. In the parallel implementation, the rays for each pixel are shot in parallel. The super-sampling rays for anti-aliasing method are also shot in parallel. Please note that unless you are using a multi-core system, the program will not 'truly' run in parallel. To build the parallel version of the program, please install OpenMP on your MacOS system and run 'make parallel'. The command to run the program are the same as the serial version.

	NOTE: Both recursion and anti-aliasing are ON by default. The still images show the difference between enabling these extra credit features and disabling them. 

	NOTE: Ensure that OpenMP is installed for the parallel program. Run 'brew install libomp' if not installed and change the PARALLEL_INCLUDE and PARALLEL_LIBRARY variables in the Makefile accordingly to point to the OpenMP libraries on your computer. These locations should be mentioned once HomeBrew finishes installing OpenMP. 


8) Instructions for running the program:
	Simply type make in the directory with the Makefile to build the program. 
	Run the program with this command:
	./assign3 SceneFiles/<scenefile> output.jpeg
	where scenefile is the name of a scene in the SceneFiles folder.
	Make sure the Pic library has been built and exists at the same directory level as this project.
