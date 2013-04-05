Summary: Data Acquisition driver for several DAQ boards
Name: comedi
Version: 0.7.63
Release: 2
Copyright: GPL
Group: System Environment/Kernel
Source: http://www.comedi.org/comedi/download/comedi-0.7.63.tgz
Patch: comedi.patch
BuildRoot: /var/tmp/%{name}-buildroot
requires: kernel = 2.4.7, kernel-source = 2.4.7
provides: comedi

%description
Comedi is a data acquisition driver for Linux.  Together
with Comedilib, it allows Linux processes to acquire data from
supported DAQ cards, such as those from National Instruments.

%prep
%setup -q
%patch -p1 -b .buildroot

%build
cp /usr/src/linux-2.4/configs/kernel-2.4.7-i686.config /usr/src/linux-2.4/.config
#make dep in the kernel
cd /usr/src/linux-2.4
make dep
#is there a better way to get back to the build directory?
cd /usr/src/redhat/BUILD/comedi-0.7.63
make


%install
#install also gets called while compiling the RPM
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/lib/modules/2.4.7-10/comedi/comedi/drivers
mkdir -p $RPM_BUILD_ROOT/lib/modules/2.4.7-10/comedi/comedi/kcomedilib
mkdir -p $RPM_BUILD_ROOT/usr/include
#comedi runs depmod which isn't in the path by default
#depmod actually needs to be run during the install process.
export PATH=$PATH:/sbin
make install
#apparently rpm requires all the files it archives to be located
#in $RPM_BUILD_ROOT, so we copy over the files we need to save
cp /lib/modules/2.4.7-10/comedi $RPM_BUILD_ROOT/lib/modules/2.4.7-10 -r
cp /usr/src/redhat/BUILD/comedi-0.7.63/include/linux/comedi.h
$RPM_BUILD_ROOT/usr/include/
cp /usr/src/redhat/BUILD/comedi-0.7.63/include/linux/comedilib.h
$RPM_BUILD_ROOT/usr/include/

%post
#post gets called on the user's system after the files have been copied over
#"make install" does:
/sbin/depmod -a
#"make dev" does:
mknod -m 666 /dev/comedi0 c 98 0
mknod -m 666 /dev/comedi1 c 98 1
mknod -m 666 /dev/comedi2 c 98 2
mknod -m 666 /dev/comedi3 c 98 3

%postun
#postun is called after the files have been uninstalled
rm -f /dev/comedi0
rm -f /dev/comedi1
rm -f /dev/comedi2
rm -f /dev/comedi3

%clean
#clean can be called after building the package

%files
%defattr(-,root,root)
/lib/modules/2.4.7-10/comedi
#/usr/include/comedi.h
#/usr/include/comedilib.h

%changelog
* Wed Feb 20 2002 Tim Ousley <tim.ousley@ni.com>
- initial build of comedi RPM

