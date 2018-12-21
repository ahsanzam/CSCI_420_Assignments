# Makefile 
# CSCI 420
# Ahsan Zaman
# Assignment 3

# we assume the pic directory locates one level above,
# change PIC_PATH if this is not the case
PIC_PATH = $(abspath $(CURDIR)/../pic)

INCLUDE = -I$(PIC_PATH)
LIBRARIES = -L$(PIC_PATH) -framework OpenGL -framework GLUT -lpicio -ljpeg -lm

COMPILER = g++
COMPILERFLAGS = -O3 $(INCLUDE) -Wno-deprecated -Wno-deprecated-declarations
PROGRAM = assign3
SOURCE = assign3.cpp
OBJECT = assign3.o


PARALLEL_FLAGS = -Xpreprocessor -fopenmp -lomp 
PARALLEL_INCLUDE = -L/usr/local/opt/libomp/lib -I/usr/local/opt/libomp/include 
PARALLEL_LIBRARY -DOpenMP_CXX_FLAGS="-Xpreprocessor -fopenmp -I/usr/local/opt/libomp/include" -DOpenMP_CXX_LIB_NAMES="omp" -DOpenMP_omp_LIBRARY=/usr/local/opt/libomp/lib/libomp.dylib

.cpp.o: 
	$(COMPILER) -c $(COMPILERFLAGS) $<

all: $(PROGRAM)

$(PROGRAM): $(OBJECT)
	$(COMPILER) $(COMPILERFLAGS) -o $(PROGRAM) $(OBJECT) $(LIBRARIES)
	# ./assign3 SceneFiles/test2.scene output.jpeg
	# rm assign3 assign3.o

parallel: $(OBJECT)
	$(COMPILER) $(COMPILERFLAGS) $(PARALLEL_FLAGS) $(PARALLEL_INCLUDE) $(PARALLEL_LIBRARY) -o parallel $(OBJECT) $(LIBRARIES)

clean:
	-rm -rf core *.o *~ "#"*"#" $(PROGRAM)