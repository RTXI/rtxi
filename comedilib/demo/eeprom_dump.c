/*
   Dumps eeproms
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <stdlib.h>
#include "examples.h"

int read_eeprom(comedi_t *it,unsigned int **eeprom, struct parsed_options options);
void dump_eeprom(unsigned int *eeprom,int len);

comedi_t *device;

int main(int argc, char *argv[])
{
	int len;
	unsigned int *eeprom;
	struct parsed_options options;

	init_parsed_options(&options);
	options.subdevice = -1;
	parse_options(&options, argc, argv);

	device = comedi_open(options.filename);
	if(!device){
		comedi_perror(options.filename);
		exit(-1);
	}

	len = read_eeprom(device, &eeprom, options);
	dump_eeprom(eeprom,len);

	return 0;
}




int read_eeprom(comedi_t *it, unsigned int **eeprom, struct parsed_options options)
{
	int n,i,ret;
	lsampl_t data;
	unsigned int *ptr;
	lsampl_t maxdata;

	if(options.subdevice < 0)
	{
		options.subdevice = comedi_find_subdevice_by_type(it, COMEDI_SUBD_MEMORY, 0);
		if(options.subdevice < 0){
			fprintf(stderr,"No memory subdevice\n");
			return 0;
		}
	}

	n = comedi_get_n_channels(it, options.subdevice);
	maxdata = comedi_get_maxdata(it, options.subdevice, 0);

	if(maxdata != 0xff){
		fprintf(stderr,"Demo only supports 8-bit memory subdevice has strange maxdata, aborting\n");
		exit(-1);
	}

	ptr = malloc(sizeof(unsigned int) * n);

	for(i = 0; i < n; i++){
		ret = comedi_data_read(it, options.subdevice, i, 0, 0, &data);
		ptr[i] = data;
		if(ret < 0){
			comedi_perror("comedi_data_read");
			return 0;
		}
	}

	*eeprom=ptr;
	return n;

}

void dump_eeprom(unsigned int *eeprom,int len)
{
	int i, j, c;

	for (i = 0; i < len - 16; i+=16) {
		printf("%04x: ",i);
		for (j = 0; j < 16; j++) {
			printf("%02x", eeprom[i + j] & 0xff);
		}
		printf("  ");
		for (j = 0; j < 16; j++) {
			c = eeprom[i + j] & 0xff;
			printf("%c", isprint(c) ? c : '.');
		}
		printf("\n");
	}
	if(i==len)return;
	printf("%04x: ",i);
	for (j = 0; j < len-i; j++) {
		printf("%02x", eeprom[i + j] & 0xff);
	}
	for(;j<16;j++){
		printf("  ");
	}
	printf("  ");
	for (j = 0; j < len-i; j++) {
		c = eeprom[i + j] & 0xff;
		printf("%c", isprint(c) ? c : '.');
	}
	for(;j<16;j++){
		printf(" ");
	}
	printf("\n");
}

