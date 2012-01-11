/*
*  osiUserCode.cpp
*
*   
*  osiUserCode.cpp holds the two user defined functions
*  needed to port iBus to a target platform.
*
*  acquireBoard( ) -- constructs and initializes the iBus
*
*  releaseBoard( ) -- deletes and cleans up the iBus
*
*  There are also assorted helper functions which are also 
*  platform specific.
*
* 
*  $DateTime: 2006/07/27 23:51:45 $
*
*/

//Platform independent headers
#include "osiBus.h"


//Linux specific headers
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
//#include <asm/page.h>
#include <sys/user.h>

#ifndef MAP_32BIT
#define MAP_32BIT 0
#endif

struct LinuxSpecific{
	u32 _fileDescriptor;
};

i32 FindPCICard(u32,u32*,u32*,u32*,u32*);
i32 MapPCIMemory(u32,u32,u32,u32,void**,void**,i32*);


/* Return iBus of first matching PCI/PXI card.
*/
iBus* acquireBoard(u32 devBAR0,u32 devBAR1)
{
	u32 temp_PCIID,devBAR0Len,devBAR1Len;
    //u32 devBAR0,devBAR1;
	i32 retval,fd;
	void *mem0,*mem1;
	iBus *bus;
	LinuxSpecific *privateIBus;

	u32 busNumber = 0; 
	u32 devNumber = 0; 
	u32 devLocation = 0;
	//sscanf (brdLocation,"PXI%d::%d::INSTR", &busNumber, &devNumber);

	//devLocation = (busNumber<<8) | ((devNumber&0x1f)<<3);
	//Find the PCI memory ranges from /proc/bus/pci/devices
	//retval=FindPCICard(devLocation,&devBAR0,&devBAR1,&devBAR0Len,&devBAR1Len);
	//if(retval<0) return NULL;
	//printf("PCI Device ID 0x%lx, BAR0 %lx-%lx, BAR1 %lx-%lx \n",devLocation,devBAR0,devBAR0Len,devBAR1,devBAR1Len);

    //devBAR0 = 0xfe7ff000;
    devBAR0Len = 4096;

    //devBAR1 = 0xfe7fe000;
    devBAR1Len = 4096;

	//Memory map /dev/mem to get access to the PCI Card's memory
	retval=MapPCIMemory(devBAR0,devBAR1,devBAR0Len,devBAR1Len,&mem0,&mem1, &fd);
	if(retval<0) return NULL;
	//printf("PCI Device ID 0x%lx, mem0 0x%lx, mem1 0x%lx\n",devLocation,mem0,mem1);

	//create a new iBus which uses the mmaped addresses of /dev/mem
	bus = new iBus(0, 0, mem0, mem1);
	bus->_physBar[0] = devBAR0;
	bus->_physBar[1] = devBAR1;
	bus->_physBar[2] = NULL;
	bus->_physBar[3] = NULL;
	bus->_physBar[4] = NULL;
	bus->_physBar[5] = NULL;
	
	privateIBus =  new LinuxSpecific;
	privateIBus->_fileDescriptor = fd;
	bus->_osSpecific = (void*)privateIBus;

	return bus;
}

void  releaseBoard(iBus *&bus)
{
	LinuxSpecific *privateIBus;
	privateIBus = (LinuxSpecific*) bus->_osSpecific;
	
	//close /dev/mem
	close(privateIBus->_fileDescriptor);

	delete privateIBus;
	delete bus;
}


/*Find the system memory ranges for a given PCI card */
i32 FindPCICard(u32 location,u32* BAR0,u32* BAR1,
										  u32* BAR0Len,u32* BAR1Len)
{
	/*In Linux 2.4.4, the file /proc/bus/pci/devices tells us the memory
		used for each PCI card.  It also tells us which interrupt
		the PCI card is assigned.
		*/
		
	int fd = open("/proc/bus/pci/devices", O_RDONLY);
	int i,retval;
	u32 currentLocation;
	char buffer[300];
	do{
		i=0;
		retval = read(fd,buffer,140);
		i += 140;
		while((buffer[i-1] != 0x0a)&&(retval>0)){
			retval = read(fd,buffer+i,1);
			i++;
		}
		buffer[i]=0;
		sscanf(buffer,"%x %*x %*x %lx %lx %*x %*x %*x %*x %*x %lx %lx",
				&currentLocation,BAR0,BAR1,BAR0Len,BAR1Len);
	}while((currentLocation != location)&&(retval>0));
	close(fd);
	if(currentLocation == location) return 0;
		else return -1;
}


/* Return pointers to the PCI card's registers.
*/
i32 MapPCIMemory(u32 BAR0, u32 BAR1, u32 BAR0Len, u32 BAR1Len,
				  void** BAR0Mem, void** BAR1Mem, i32 *mmfd)
{
	
	/*In Linux 2.4.4, the file /dev/mem can be memory mapped
		to access system memory.  Since the PCI cards show up in system
		memory, we can access the PCI cards through /dev/mem.
		*/

	/*Open /dev/mem */
	if ((*mmfd = open("/dev/mem", O_RDWR) ) < 0) {
		printf("iBus: can't open /dev/mem \n");
		return -1;
	}
	
	
	/*Get BAR1 */
	/*Get a Page aligned buffer */
	if ((*BAR1Mem = malloc(BAR1Len + (PAGE_SIZE-1))) == NULL) {
		printf("iBus: allocation error \n");
		return -1;
	}

	if ((ptr_uint_t)*BAR1Mem % PAGE_SIZE)
		*BAR1Mem = (void*)((ptr_uint_t)*BAR1Mem + PAGE_SIZE - ((ptr_uint_t)*BAR1Mem % PAGE_SIZE));

	/*attach the device to a user address space */
	*BAR1Mem = (unsigned char *)mmap(
		(caddr_t)*BAR1Mem, 
		BAR1Len,
		PROT_READ|PROT_WRITE,
		MAP_SHARED|MAP_FIXED|MAP_32BIT|MAP_LOCKED,
		*mmfd, 
		(ptr_uint_t)BAR1
	);
	 
	if ((long)*BAR1Mem < 0) {
		printf("iBus: mmap error \n");
		return -1;
	}

	
	/*Get BAR0 */
	/*Get a Page aligned buffer */
	if ((*BAR0Mem = malloc(BAR0Len + (PAGE_SIZE-1))) == NULL) {
		printf("iBus: allocation error \n");
		return -1;
	}

	if ((ptr_uint_t)*BAR0Mem % PAGE_SIZE)
		*BAR0Mem = (void*)((ptr_uint_t)*BAR0Mem + PAGE_SIZE - ((ptr_uint_t)*BAR0Mem % PAGE_SIZE));

	/*attach the device to a user address space */
	*BAR0Mem = (unsigned char *)mmap(
		(caddr_t)*BAR0Mem, 
		BAR0Len,
		PROT_READ|PROT_WRITE,
		MAP_SHARED|MAP_FIXED|MAP_32BIT|MAP_LOCKED,
		*mmfd, 
		(ptr_uint_t)BAR0
	);
	 
	if ((long)*BAR0Mem < 0) {
		printf("iBus: mmap error \n");
		return -1;
	}
	return 0;
}

/*
* DMA Memory support
*/
tDMAMemory * iBus::allocDMA (u32 size)
{ 
	printf("dma alloc not supported\n"); 
	return NULL;
}

void iBus::freeDMA (tDMAMemory *mem)
{}
