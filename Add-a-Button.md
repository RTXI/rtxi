This tutorial provides a guideline for adding buttons within an RTXI module. We will be using my_plugin_gui for starter code, and this tutorial will focus on adding a third button to the existing group of two.

Before | After
:-----:|:-----:
[[/images/add_a_button/1_module_before.png]] | [[/images/add_a_button/2_module_after.png]]

Before we begin, please make sure that your RTXI installation is less than version 2.0. Versions 2.0 and above rely on Qt4. This tutorial utilizes code that is compatible with Qt3 libraries and should not be expected to provide error-free code if used as a starting point for plugins written for Qt4. 

####Preparation:
1. Create a new directory and call it add_button.  
   `$ mkdir add_button`

2. Copy the files from my_plugin_gui and place them in add_button.  
   `$ cp ~/modules/my_plugin_gui/* ~/PATH_TO/add_button/`

3. Move to the directory and change the filenames my_plugin_gui.cpp and my_plugin_gui.h to add_button.cpp and add_button.h, respectively.  
   `$ mv my_plugin_gui.cpp add_button_.cpp`  
   `$ mv my_plugin_gui.h add_button_.h`  

4. Alter the makefile, source file, and header file to reflect the file name changes.  
   Run the line below within the add\_button directory. The sed command searches for all user-specified files for a string that, if found, is replaced by another user-specified string. The command below will search the directory for all instances of "my_plugin_gui" and replace them with "add_button".  
   `$ sed -i "s/my_plugin_gui/add_button/g" Makefile add_button.cpp add_button.h`  

Before | After
:-----:|:-----:
[[/images/add_a_button/3_makefile_before_sed.png]] | [[/images/add_a_button/4_makefile_after_sed.png]]

####Header File:
1. Open add_button.h. The screenshots in the tutorial all use vim, but feel free to use whatever text editor or IDE you prefer.  

2. Within the "private slots" section of the file, add a function called cBttn_event.  
   `void cBttn_event(void);`  

   This function will be associated with the button we will create in the GUI. In other words, the function, whose contents will be defined in the source file, will be called each time the button we will associate it with is pressed.  

3. Save your changes and close the file.

Before | After
:-----:|:-----:
[[/images/add_a_button/5_my_plugin_gui_h.png]] | [[/images/add_a_button/6_add_button_h.png]]

####Source File:
1. Open add_button.cpp.

2. Go to around line 107 and begin a new line. Enter:  
   `QPushButton cBttn = new QPushButton("Button C", bttnGroup);`  

   QPushButton is a Qt class that creates a button that undergoes an animation when clicked that gives the appearance of it being pushed down and back, like a real-life button. The initialization takes two arguments:  
   1. A string that will be displayed on the button on the GUI. The button's label, essentially.  
   2. The parent object for the button.  

   In our case, cBttn is a new QPushButton that is a child of the bttnGroup widget. If you read a few lines above, you can see bttnGroup's initialization. bttnGroup is a QHButtonGroup object, meaning that it is a Qt-defined widget that puts buttons in a horizontal row in the order in which they are added. By default, QHButtonGroup handles button spacing, widget size, etc., but behavior can also be specified by the user. This tutorial will not cover that. 

3. Skip a few lines down to around line 113. Add the following:  
   `QObject::connect(cBttn, SIGNAL(clicked()), this, SLOT(cBttn_event()));}`

   This line connects the clicking of cBttn to the function cBttn_event. The connect function's namespace is not included in the file or the header, so the "QObject::" prefix is necessary. The arguments are, in sequential order:
   1. The name of the object to be connected (cBttn).  
   2. The signal type (SIGNAL(type of signal)) sent by the object.  
   3. The Qt object that receives the signal, which in this case is the object itself, so 'this' is used.  
   4. The response of the receiving object (call cBttn_event). SIGNAL()s are received by SLOT()s, so the expression needed is SLOT(cBttn_event()).

Before | After
:-----:|:-----:
[[/images/add_a_button/7_my_plugin_gui_cpp1.png]] | [[/images/add_a_button/8_add_button_cpp1.png]]

4.. Skip to the very bottom of the file and define the cBttn_event function. You can use whatever you like, but our main goals is to simply create a button. Therefore, the function definition can be left empty. This will result in button clicks not doing anything.  
   `void cBttn_event(void) {};`  
   
   cBttn_event will still be called whenever a click occurs. We simply have made it so each call to cBttn_event will not do anything.  

[[/images/add_a_button/9_add_button_cpp2.png]]

5.. Save changes and close the file.  

####Compilation: 
1. Compile the plugin from within the add_button directory. Run:  
   `$ sudo make install`
   
   This will compile and install the software. If a compilation error occurs, check the source files for types. This is a simple plugin, so complex errors are not to be expected.  

[[/images/add_a_button/10_compilation.png]]

####Running in RTXI:
1. Open RTXI.  
2. Go the the top menubar and select "Modules" from the menu. It is second from the left. 
3. Go to "Load user module".  
4. Select your module from the list of available ones. It will be named "add_button.so".
5. View the new button.

[[/images/add_a_button/11_module_in_rtxi.png]]

####Concluding Remarks:
This completes the button-adding tutorial. Granted, there are many options to explore that have not been explained or addressed. Development for RTXI is ongoing, and user feedback on this tutorial and/or suggestions for future ones is greatly appreciated and will be addressed with due speed.