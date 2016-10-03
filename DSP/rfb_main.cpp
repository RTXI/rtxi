#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <iostream> 
#include <fstream>
#include <math.h>
#include "misdefs.h"   

#ifdef _DEBUG
 ofstream DebugFile("raw_file.bug", ios::out);
#endif

main()
{
  FILE *in_file, *out_file;
  char input_file_name[25];
  char token[10];
  char dump;
  int indx2, indx3;
  unsigned int hex_value;
  int dec_value;
  int samp_cnt, skip_cnt, num_samps, decim_rate;

  printf( "welcome to raw file browser\n");
  printf( "enter name of file to browse\n");
  gets(input_file_name);

  in_file = fopen(input_file_name, "rb");
  out_file = fopen("Ace.txt", "w");

  samp_cnt = -1;
  printf("number of bytes to skip over?\n");
  scanf("%d", &skip_cnt);
  printf("skip_cnt = %d\n", skip_cnt);
  printf("number of samples to convert?\n");
  scanf("%d", &num_samps);
  printf("num_samps = %d\n", num_samps);
  printf("input decimation rate?\n");
  scanf("%d", &decim_rate);
  printf("decim_rate = %d\n", decim_rate);
  
  //for(indx3=0; indx3<104000; indx3++)
  //for(indx3=0; indx3<65000; indx3++)
  for(indx3=0; indx3<skip_cnt; indx3++)
  //for(indx3=0; indx3<81000; indx3++)
    {
    dump=getc(in_file);
    }
  for(indx3=0; indx3<num_samps; indx3++)
    {
    token[0] = getc(in_file);
    token[1] = getc(in_file);
    hex_value = (unsigned int)token[1];
    hex_value &=0xff;
    hex_value <<= 8;
    hex_value |= (0xff & token[0]);
    dec_value = hex_value;
    if(dec_value > 0x7fff) dec_value -= 0x10000;
    //hex_value &= 0xffff;

   // if( indx3>=12 )
    //if( indx3>=29 )
      //{
      samp_cnt++; 
      fprintf(out_file,"%d, %d \n", samp_cnt, dec_value);
      //fprintf(out_file,"%d \n", dec_value);
      //}
    for(indx2=1; indx2<decim_rate; indx2++)
      {
      token[0] = getc(in_file);
      token[1] = getc(in_file);
      }
    }

  //pgm_exit:
  fclose(in_file);
  return 0;
}  
