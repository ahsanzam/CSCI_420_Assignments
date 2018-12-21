CSCI 420 Fall 2018
Prof. Hao Li 
	Ahsan Zaman
	ahsanzam@usc.edu
	Oct 29, 2018

Assignment 2

TO RUN
1. Please navigate to the `assign2` directory and run `make`. This will build the program and also run it.
2. If you test with different size tracks, you may have to adjust the MOVE_INTERVAL and TRACK_INTERVAL to get a good speed for movement and good spacing between the wooden slats placed at regular intervals. Ensure TRACK_INTERVAL < MOVE_INTERVAL or it will look like you are moving backward even though you are actually moving forward. 

PLEASE NOTE: 
1. I have rendered double side rails for the track as well as a 'floor' section all texture mapped with a metal texture.
2. I have rendered the wooden texture mapped bar cross section at regular intervals for realism. 
3. There are no visible seams in my skybox.  
4. I used OpenGL display lists to draw my tracks and skybox for fast and efficient rendering. 