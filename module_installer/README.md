### Module Installer

**Requirements:** libgit2-dev, libmarkdown2-dev, -lQt5Network  
**Limitations:** testing  

![Module Installer GUI](module-installer.png)

<!--start-->
Download and install modules within RTXI. If RTXI is running as root, all modules will be downloaded and installed into `/usr/local/lib/rtxi_modules`, and if not, they will be found in `~/.config/rtxi/`.  

The module can only keep track of modules installed in the above pre-defined directories.  

<!--end-->

To-do list:

 - Add progress bar for cloning/installing/upgrading.  
 - Notify users if updates are available for installed modules.  

Known bugs: 

 - READMEs don't get updated after installing/updating a module. 
 - possible crashes when the "download/update" button is hit
