# Makefile 
# CSCI 480
# Assignment 1

# we assume the pic directory locates one level above,
# change PIC_PATH if this is not the case
PIC_PATH = $(abspath $(CURDIR)/../pic)

INCLUDE = -I$(PIC_PATH)
LIBRARIES = -L$(PIC_PATH) -framework OpenGL -framework GLUT -lpicio -ljpeg
HEIGHT_MAP_DIR = height_maps
COMPILER = g++
COMPILERFLAGS = -O3 $(INCLUDE)

PROGRAM = assign1
SOURCE = assign1.cpp
OBJECT = assign1.o

.cpp.o: 
	$(COMPILER) -c $(COMPILERFLAGS) $< -Wno-deprecated -Wno-deprecated-declarations

all: $(PROGRAM)

$(PROGRAM): $(OBJECT)
	$(COMPILER) $(COMPILERFLAGS) -o $(PROGRAM) $(OBJECT) $(LIBRARIES) 
	rm ./*.o
	# ./$(PROGRAM) $(HEIGHT_MAP_DIR)/flower.jpeg
	# ./$(PROGRAM) $(HEIGHT_MAP_DIR)/SantaMonicaMountains-128.jpg
	# ./$(PROGRAM) $(HEIGHT_MAP_DIR)/OhioPyle-768.jpg
	# ./$(PROGRAM) $(HEIGHT_MAP_DIR)/GrandTeton-256.jpg
	# ./$(PROGRAM) $(HEIGHT_MAP_DIR)/GrandTeton-512.jpg

clean:
	-rm -rf *.o *~ .*~ $(PROGRAM)
