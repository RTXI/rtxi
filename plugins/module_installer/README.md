### Module Installer

**Requirements:** libgit2-dev, libmarkdown2-dev, -lQt5Network  
**Limitations:** testing  

![Module Installer GUI](module-installer.png)

<!--start-->
Download and install modules within RTXI. If RTXI is running as root, all modules will be downloaded and installed into `/usr/local/lib/rtxi_modules`, and if not, they will be found in `~/.config/rtxi/`.  

The module can only keep track of modules installed in the above pre-defined directories.  
<!--end-->

Also, note that this module uses GitHub's API to query all the repos on our GitHub account. GitHub limits the number of requests that can be made to 60 per hour. Any more will cause your IP address to be blocked until the hour has passed. If this happens, it will look like the "Sync Repos" button stopped working. This only affects the "Sync Repos" button. This module gets webpages directly for the READMEs, so you can download them without being cut off.  

To-do list:

 - Add progress bar for cloning/installing/upgrading.  
 - Notify users if updates are available for installed modules.  

