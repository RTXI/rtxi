RTXI
====

Real-Time eXperiment Interface is an extensible hard realtime platform for the development of novel experiment control and signal processing applications.

========================================================================================
Building & installing RTXI
========================================================================================
These commands will compile RTXI. This is all you need to do if you are updating
RTXI. Enter your RTXI source code directory:

  $ sh autogen.sh
  $ sudo ./configure
  $ sudo make
  $ sudo make install
  $ sudo cp rtxi.conf /etc

========================================================================================
Building & installing DYNAMO
========================================================================================
If you need DYNAMO, you will need to compile the DYNAMO trans-
lation utility.

  $  sudo apt-get install mlton
  $  cd /rtxi/dynamo
  $  mllex dl.lex
  $  mlyacc dl.grm
  $  mlton dynamo.mlb
  $  sudo cp dynamo /usr/bin


Follow the instructions below to compile a new real-time Linux kernel and the RTXI
dependencies. Then come back to install RTXI.

========================================================================================
Building a real-time Ubuntu linux kernel
========================================================================================

1. Install a clean version of Ubuntu. Update and get the following required packages.

   $ sudo apt-get update
   $ sudo apt-get upgrade
   $ sudo apt-get install cvs subversion build-essential
   $ sudo apt-get install kernel-package linux-source
   $ sudo apt-get install libncurses5-dev libtool automake
   $ sudo apt-get install bison flex qt4-dev-tools
   $ sudo apt-get install libboost-dev libgsl0-dev
   $ sudo apt-get install install libboost-program-options-dev
   $ sudo apt-get install libhdf5-serial-1.8.4

2. Download the latest stable version of RTAI 3.x at www.rtai.org.
   For the sake of this demonstration I'll be using RTAI version 3.8.1.

   $ cd /opt
   $ sudo wget --no-check-certificate https://www.rtai.org/RTAI/rtai-3.8.1.tar.bz2
   $ sudo tar xjvf rtai-3.8.1.tar.bz2

3. Determine the latest supported Linux kernel.

   $ cd /opt/rtai-3.8.1/base/arch/x86/patches
   $ ls

    You should see several files with names of the form:
      hal-linux-#-x86-#.patch
    The kernel version is the first #, and should be something like 2.6.31.8.

4. Download the version of the Linux kernel determined in the previous step
   from www.kernel.org/pub/linux/kernel/v2.6, e.g. linux-2.6.31.8.tar.bz2

   $ cd /usr/src
   $ sudo wget http://www.kernel.org/pub/linux/kernel/v2.6/linux-2.6.31.8.tar.bz2
   $ sudo tar xjvf linux-2.6.31.8.tar.bz2

5. Apply the RTAI patch to the Linux kernel source. If you see messages about hunks 
   failing, the patch did not correctly match your Linux kernel version. You should 
   re-extract the correct Linux kernel and try again with the correct RTAI patch.

   $ cd /usr/src/linux-2.6.31.8
   $ sudo patch -p1 < /opt/rtai-3.8.1/base/arch/x86/patches/hal-linux-2.6.31.8-x86-2.4-09.patch

6. Configure the Linux kernel source. The easiest technique is to use an existing 
   configuration as a base, then make some minor modifications. The location of existing 
   configurations varies from distribution to distribution, but /boot seems to be a 
   fairly popular place, try running:

   $ cd /usr/src/linux
   $ sudo cp /boot/config-`uname -r`.config
   $ sudo make oldconfig

      N.B.  The ` in the second command is not a regular single qoute, but rather
            the kind on the ~ (tilde) key.
      N.B.  When you run the third command you will probably be prompted concerning 
            a bunch of different configuration options... just press enter for each.

   Given a base configuration you need to make some minor tweaks. Open a menu to make 
   these additional changes. If you do not see "ADEOS" or "Interrupt pipeline" or 
   "IPIPE", your patch did not work. Below is a sample kernel configuration. Not all of 
   the options that are listed here may appear in your menu. You should configure those
   options that you do see as shown. Kernel versions 2.6.30 and later have an 
   experimental in-tree COMEDI implementation in the driver ”staging” area. RTAI 
   support for the in-tree ”staging” version is still in development, so for current 
   RTAI releases the Git version of COMEDI should be used. To avoid module inconsistencies 
   later, you may want to opt out of compiling the COMEDI kernel staging drivers.

   $ sudo make menuconfig

     o General setup
       - Set "Local version - append to kernel release" to "-adeos"
         [or whatever you want]
     o Enable loadable module support
        - [∗] Module unloading
        - [ ] Module versioning support
     o Processor type and features
        - [∗] Symmetric multi-processing support
        - [ ] Support sparse irq numbering
        - Processor family to Pentium-4/Celeron/Pentium-4 M/older Xeon
          [or whatever processor type you have]
        - Preemption Model ---> Preemptible Kernel (Low-Latency Desktop)
        - [∗] Interrupt pipeline
        - [∗] Local APIC support on uniprocessors
           - [∗] IO-APIC support on uniprocessors
     o Power management and ACPI options
        - [∗] Power Management support
        - [ ] Suspend to RAM and standby
        - [ ] Hibernation
        - [∗] ACPI (Advanced Configuration and Power Interface) Support
           - [ ] AC Adapter
           - [ ] Battery
           - [ ] Button
           - [*] Fan
           - [*] Dock
           - [*] Processor
           - [*] Thermal Zone
           - [*] Smart Battery System
        - CPU Frequency scaling
           - [ ] CPU Frequency scaling
     o Device Drivers
        - [∗] Staging drivers
           - [ ] Data acquisition support (comedi)
     o Kernel hacking
        - [ ] KGDB: kernel debugging with remote gdb

7. Build the real-time Linux kernel.

   $ sudo make-kpkg clean
   $ sudo make-kpkg --initrd kernel image kernel headers kernel source

8. Install the real-time Linux kernel.

   $ cd /usr/src
   $ sudo dpkg -i linux-headers-2.6.31.8-adeos 2.6.31.8-adeos-10.00.Custom i386.deb
   $ sudo dpkg -i linux-image-2.6.31.8-adeos 2.6.31.8-adeos-10.00.Custom i386.deb

9. Reboot into the new Linux kernel. Reboot the system and select the new RT
   kernel from the list provided.

========================================================================================
Building & installing RTAI
========================================================================================

10. Configure RTAI without COMEDI support.

    $ cd /opt/rtai-3.8.1
    $ sudo make menuconfig

     o General
       - Installation ---> /usr/realtime
       - Kernel source ---> /usr/src/linux
     o Machine
       - (#) Number of CPUs (SMP-only)
     o Other Features
       - [∗] User space interrupts
     o Add-ons
       - [ ] Real Time COMEDI support in user space

11. Build & Install RTAI.

    $ sudo make
    $ sudo make install

========================================================================================
Building & installing comedi
========================================================================================

12. Download the latest version of COMEDI.

    $ sudo apt-get install git-core
    $ cd /opt
    $ sudo git clone git://comedi.org/git/comedi/comedi.git
    $ sudo git clone git://comedi.org/git/comedi/comedilib.git
    $ sudo git clone git://comedi.org/git/comedi/comedi_calibrate.git

13. Extract and install the COMEDI libraries.

    $ cd /opt/comedilib
    $ sudo sh autogen.sh
    $ sudo ./configure
    $ sudo make
    $ sudo make install

    $ cd /opt/comedi calibrate
    $ sudo autoreconf -i
    $ sudo ./configure
    $ sudo make
    $ sudo make install

    $ cd /opt/comedi
    $ sudo sh autogen.sh
    $ sudo ./configure
    $ sudo make
    $ sudo make install

========================================================================================
Building & installing RTAI redux
========================================================================================

14. Configure RTAI WITH COMEDI support.

    $ cd /opt/rtai-3.8.1
    $ sudo make menuconfig

     o General
       - Installation ---> /usr/realtime
       - Kernel source ---> /usr/src/linux
     o Machine
       - (#) Number of CPUs (SMP-only)
     o Other Features
       - [∗] User space interrupts
     o Add-ons
       - [*] Real Time COMEDI support in user space
         - COMEDI installation directory ---> /opt/comedi

15. Build & Install RTAI.

    $ sudo make
    $ sudo make install

========================================================================================
Automating RTAI kernel modules
========================================================================================

16. To use RTAI, you must have the necessary RTAI and COMEDI kernel modules loaded. 
    Create these 2 files in /usr/realtime/bin to load all the modules we need for RTXI. 
    Use any text editor you like and copy-paste the lines given. The *DRIVER* is the 
    COMEDI driver name that is appropriate for your DAQ card. For example, most National 
    Instruments multifunction DAQ cards use the driver "ni_pcimio". Refer to 
    http://www.comedi.org/hardware.html.


    /usr/realtime/bin/rtaiinsmod
    -------------------------------------------------------------------------
    # Inserts RTAI and COMEDI modules in kernel and configures the drivers.
    insmod /usr/realtime/modules/rtai_hal.ko
    insmod /usr/realtime/modules/rtai_lxrt.ko
    insmod /usr/realtime/modules/rtai_sem.ko
    insmod /usr/realtime/modules/rtai_shm.ko
    modprobe kcomedilib
    insmod /usr/realtime/modules/rtai_comedi.ko
    # Hardware dependent lines below.
    modprobe *DRIVER*
    -------------------------------------------------------------------------


    /usr/realtime/bin/rtairmmod
    -------------------------------------------------------------------------
    # Inserts RTAI and COMEDI modules in kernel and configures the drivers.
    insmod /usr/realtime/modules/rtai_hal.ko
    insmod /usr/realtime/modules/rtai_lxrt
    insmod /usr/realtime/modules/rtai_sem.ko
    insmod /usr/realtime/modules/rtai_shm.ko
    modprobe kcomedilib
    insmod /usr/realtime/modules/rtai_comedi.ko
    # Hardware dependent lines below.
    modprobe *DRIVER*
    -------------------------------------------------------------------------

17. Make these scripts executable.

    $ sudo chmod +x /usr/realtime/bin/rtaiinsmod
    $ sudo chmod +x /usr/realtime/bin/rtairmmod

18. Now lets execute your script automagically when you boot the kernel. Open 
    /etc/rc.local for editing. Add a line to the end before the “exit 0”:

    -------------------------------------------------------------------------
    #!/bin/sh -e
    #
    # rc.local
    #
    # This script is executed at the end of each multiuser runlevel.
    #
    # By default this script does nothing.
    /usr/realtime/bin/rtaiinsmod
    exit 0

    When you reboot, the kernel modules in rtaiinsmod should be loaded. You can
    check this by:

    $ lsmod
