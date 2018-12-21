CSCI 420 Project 1 
Height Maps
Ahsan Zaman
September 24, 2018

To Run: 
Navigate to the 'assign1' directory and type 'make' into the command line. To start the program, type './assign1 height_maps/' and append the desired heightmap filename. For example: './assign1 height_maps/flower.jpeg'. Heightmaps can be found in the folder titled 'height_maps'. 

Points to Note:
1. You can use either 't' or 'control' keys for translation and 's' or shift for scaling. This is useful on Mac, where the shift key is not recognized as a GLUT modifier.
2. To switch between points, wireframe, and solid, right-click to bring up the menu. 
3. For images with high pixel density, the 'points' and 'wireframe' modes look as if they are the same as on 'solid' mode. However, if you zoom in, you will notice the image is not solid, but the vertices are simply very close together. 
4. For the screen capture portion of the assignment, I create a do_record and autorotate function to automate the changes in the recording. To activate, set the 'record' boolean variable to 'true' on line 17 of assign1.cpp. You can also activate the autorotate function by selecting 'Autorotate' from the right-click menu.
5. Two screenshots sets have been provided because I was not sure if Prof. Li wanted the spiral model in the screenshot. 'Screenshots_Flower' contains screenshots from a flower heightmap I downloaded online and 'Screenshots_Spiral' contains the screenshots for the standard spiral.

Extra Credit 
1. Wireframe overlay has been implemented. To toggle activation, choose 'Overlay' from the right-click menu. Note that you have the option to activate the overlay over either the points, wireframe, or solid drawing of the object. 
2. Textures have been implemented. To toggle a grass texture on the object, select the 'Texture' option from the right-click menu.
3. Color mapping has been implemented. To toggle on/off, select color from the right click menu. If the a color map file for the input image dimensions does not exist in the extras folder, color mapping will be disabled.

Note that activating wireframe overlay or textures automatically changes rendermode to SOLID. 