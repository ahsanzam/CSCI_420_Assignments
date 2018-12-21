# Makefile 
# CSCI 480
# Assignment 2

# we assume the pic directory locates one level above,
# change PIC_PATH if this is not the case
PIC_PATH = $(abspath $(CURDIR)/../pic)

INCLUDE = -I$(PIC_PATH)
LIBRARIES = -L$(PIC_PATH) -framework OpenGL -framework GLUT -lpicio -ljpeg -lm

COMPILER = g++
COMPILERFLAGS = -O3 $(INCLUDE) -Wno-deprecated -Wno-deprecated-declarations

PROGRAM = assign2
SOURCE = assign2.cpp
OBJECT = assign2.o

.cpp.o: 
	$(COMPILER) -c $(COMPILERFLAGS) $<

all: $(PROGRAM)

$(PROGRAM): $(OBJECT)
	$(COMPILER) $(COMPILERFLAGS) -o $(PROGRAM) $(OBJECT) $(LIBRARIES)
	./assign2 ./track.txt
	rm assign2 assign2.o

clean:
	-rm -rf core *.o *~ "#"*"#" $(PROGRAM)