
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
#include <sys/mman.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>

#include "comedi_test.h"

/* XXX this should come from elsewhere */
#define PAGE_SIZE 4096

#define N_SAMPLES 10000

sigjmp_buf jump_env;

void segv_handler(int num)
{
	siglongjmp(jump_env,1);
}

int test_segfault(void *memptr)
{
	volatile char tmp;
	int ret;
	struct sigaction act;
	struct sigaction oldact;

	memset(&act,0,sizeof(act));
	act.sa_handler=&segv_handler;
	ret = sigaction(SIGSEGV,&act,&oldact);
	if(ret)
	{
		fprintf(stderr, "sigaction failed\n");
		return 0;
	}
	ret=sigsetjmp(jump_env, 1);
	if(!ret) tmp = *((char *)(memptr));
	sigaction(SIGSEGV,&oldact,NULL);
	return ret;
}

int test_mmap(void)
{
	comedi_cmd cmd;
	unsigned char *buf;
	unsigned int chanlist[1];
	int go;
	int fails;
	int total=0;
	int ret;
	void *b;
	unsigned char *adr;
	unsigned char *map;
	unsigned int flags;
	int i;
	unsigned sample_size;
	unsigned map_len;

	flags = comedi_get_subdevice_flags(device,subdevice);

	if(!(flags&SDF_CMD) || (comedi_get_read_subdevice(device)!=subdevice)){
		printf("not applicable\n");
		return 0;
	}
	if(flags & SDF_LSAMPL) sample_size = sizeof(lsampl_t);
	else sample_size = sizeof(sampl_t);
	map_len = sample_size * N_SAMPLES;

	if(comedi_get_cmd_generic_timed(device, subdevice, &cmd, 1, 1)<0){
		printf("E: comedi_get_cmd_generic_timed failed\n");
		return 0;
	}

	buf=malloc(sample_size * N_SAMPLES);

	map = mmap(NULL, map_len,PROT_READ, MAP_SHARED, comedi_fileno(device),0);
	if(!map){
		printf("E: mmap() failed\n");
		return 0;
	}

	/* test readability */
	for(adr = map; adr < map + map_len; adr += PAGE_SIZE){
		ret=test_segfault(adr);
		if(ret){
			printf("E: %p failed\n",adr);
		}else{
			printf("%p ok\n",adr);
		}
	}

	if(realtime)cmd.flags |= TRIG_RT;

	cmd.chanlist = chanlist;
	cmd.scan_end_arg = 1;
	cmd.stop_arg = N_SAMPLES;
	cmd.chanlist_len = 1;
	chanlist[0] = CR_PACK(0,0,0);

	comedi_command(device,&cmd);

	go=1;
	b=buf;
	while(go){
		ret = read(comedi_fileno(device), b, N_SAMPLES * sample_size);
		if(ret<0){
			if(errno==EAGAIN){
				usleep(10000);
			}else{
				go = 0;
				perror("read");
			}
		}else if(ret==0){
			go = 0;
		}else{
			total += ret;
			b += ret;
			if(verbose) printf("read %d %d\n",ret,total);
		}
	}

	fails = 0;
	for(i=0;i<total;i++){
		if(buf[i]!=map[i]){
			if(fails==0)printf("E: mmap compare failed\n");
			printf("offset %d (read=%02x mmap=%02x)\n",i,
				buf[i], map[i]);
			go = 0;
			fails++;
			if(fails>10)break;
		}
	}
	if(fails==0) printf("compare ok\n");

	munmap(map, map_len);

	/* test if area is really unmapped */
	for(adr = map; adr < map + map_len; adr += PAGE_SIZE){
		ret=test_segfault(adr);
		if(ret){
			printf("%p segfaulted (ok)\n",adr);
		}else{
			printf("E: %p still mapped\n",adr);
		}
	}

	free(buf);

	return 0;
}

