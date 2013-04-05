
#define _COMEDILIB_DEPRECATED

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>

#include "comedi_test.h"


int test_mode0_read(void)
{
	comedi_trig it;
	sampl_t data;
	unsigned int chanspec;
	int save_errno;
	int ret;

	printf("rev 1\n");

	memset(&it,0,sizeof(it));
	it.subdev = subdevice;
	it.mode = 0;
	it.n_chan = 1;
	it.chanlist = &chanspec;
	it.data = &data;
	it.n = 1;

	chanspec = CR_PACK(0,0,0);

	ret = comedi_trigger(device,&it);
	save_errno = errno;

	printf("comedi_trig_ioctl: %d\n",ret);
	if(ret<0){
		printf("W: comedi_trig_ioctl: errno=%d %s\n",save_errno,
			strerror(save_errno));
	}

	return 0;
}

