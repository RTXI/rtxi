
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


int test_insn_read(void)
{
	comedi_insn it;
	lsampl_t data = 0xffffffff;
	int ret;
	int type;

	printf("rev 1\n");

	type = comedi_get_subdevice_type(device,subdevice);

	memset(&it,0,sizeof(it));
	it.subdev = subdevice;
	it.insn = INSN_READ;
	it.n = 1;
	it.chanspec = CR_PACK(0,0,0);
	it.data = &data;

	ret = comedi_do_insn(device,&it);

	if(type==COMEDI_SUBD_UNUSED){
		if(ret<0){
			if(errno==EIO){
				printf("comedi_do_insn: EIO, good\n");
			}else{
				printf("E: comedi_do_insn: %s\n",
					strerror(errno));
			}
		}else{
			printf("E: comedi_do_insn: returned %d, expected error\n",
				ret);
		}
	}else{
		if(ret<0){
			printf("E: comedi_do_insn: %s\n",strerror(errno));
		}else if(ret==1){
			printf("comedi_do_insn returned 1, good\n");
		}else{
			printf("E: comedi_do_insn returned %d\n",ret);
		}
	}

	return 0;
}

/*
 * This function tests reading with n=0.
 */
int test_insn_read_0(void)
{
	comedi_insn it;
	lsampl_t data = 0xffffffff;
	int ret;
	int type;

	type = comedi_get_subdevice_type(device,subdevice);
	if(type==COMEDI_SUBD_UNUSED){
		printf("not applicable\n");
		return 0;
	}

	memset(&it,0,sizeof(it));
	it.subdev = subdevice;
	it.insn = INSN_READ;
	it.n = 0;
	it.chanspec = CR_PACK(0,0,0);
	it.data = &data;

	ret = comedi_do_insn(device,&it);

	if(ret<0){
		printf("E: comedi_do_insn: %s\n",strerror(errno));
	}else if(ret==0){
		printf("comedi_do_insn returned 0, good\n");
	}else{
		printf("E: comedi_do_insn returned %d\n",ret);
	}

	return 0;
}

