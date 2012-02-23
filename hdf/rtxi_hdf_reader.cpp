/*
 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornel Medical College

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <cstdlib>
#include <errno.h>
#include <getopt.h>

#include <hdf5.h>
#include <hdf5_hl.h>

#include <sstream>
#include <string>

struct options {
    int info;
    int trial;
    int binary;
    int cols_start;
    int cols_end;
    int cols_step;
    int rows_start;
    int rows_end;
    int rows_step;
    char *filename;
};

int getopts(int,char *[],struct options *);
void print_trial_info(hid_t,int);

int main(int argc,char *argv[]) {
    struct options opts = {
        0,
        1,
        0,
        0,
        -1,
        1,
        0,
        -1,
        1,
        NULL,
    };

    getopts(argc,argv,&opts);

    H5Eset_auto2(H5E_DEFAULT,NULL,NULL);

    hid_t fid = H5Fopen(opts.filename,H5F_ACC_RDONLY,H5P_DEFAULT);
    if(fid < 0) {
        fprintf(stderr,"Failed to open %s.\n",opts.filename);
        return fid;
    }

    if(opts.info) {
        // count the number of trials in the file        
        hid_t trial;
        int num_trials = 0;
        std::stringstream trial_name;

        for(;;) {
            trial_name.str("");
            trial_name << "/Trial" << num_trials+1;

            if((trial = H5Gopen(fid,trial_name.str().c_str(),H5P_DEFAULT)) < 0)
                break;
            else {
                print_trial_info(trial,++num_trials);
                H5Gclose(trial);
            }
        }

        H5Fclose(fid);
        return 0;
    }

    std::stringstream data_name;
    data_name << "/Trial" << opts.trial << "/Synchronous Data/Channel Data";

    hid_t table = H5Dopen(fid,data_name.str().c_str(),H5P_DEFAULT);
    if(table < 0) {
        fprintf(stderr,"Requested trial #%d does not exist.\n",opts.trial);
        return table;
    }

    hsize_t ncols = H5Tget_size(H5Dget_type(table))/sizeof(double);
    H5Dclose(table);

    table = H5PTopen(fid,data_name.str().c_str());
    hsize_t nrows;
    H5PTget_num_packets(table,&nrows);

    // validate column and row ranges
    if(opts.cols_end == -1 || opts.cols_end >= ncols)
        opts.cols_end = ncols-1;
    if(opts.rows_end == -1 || opts.rows_end >= nrows)
        opts.rows_end = nrows-1;
    opts.cols_end -= (opts.cols_end-opts.cols_start)%opts.cols_step;
    opts.rows_end -= (opts.rows_end-opts.rows_start)%opts.rows_step;
    if((opts.cols_start-opts.cols_end)*opts.cols_step > 0)
        opts.cols_step *= -1;
    if((opts.rows_start-opts.rows_end)*opts.rows_step > 0)
        opts.rows_step *= -1;

    int row_idx = opts.rows_start;
    do {

        H5PTset_index(table,row_idx);

        double data[ncols];
        H5PTget_next(table,1,data);

        int col_idx = opts.cols_start;
        do {
            if(opts.binary) {
                write(1,data+col_idx,sizeof(double));
            } else {
                printf("%e ",data[col_idx]);
            }

            if(col_idx == opts.cols_end)
                break;
            col_idx += opts.cols_step;
        } while(1);

        if(!opts.binary)
            printf("\n");

        if(row_idx == opts.rows_end)
            break;
        row_idx += opts.rows_step;
    } while(1);

    H5PTclose(table);
    H5Fclose(fid);

    return 0;
}

int validate_range_string(const char *s) {
    std::string string = s;

    int idx = 0;
    int ndelim = 0;
    for(;;) {
        idx = string.find(':',idx+1);
        if(idx < string.length())
            ++ndelim;
        else
            break;
    };

    if(ndelim == 0) {
        for(int i=0;i<string.length();++i)
            if(!isdigit(s[i])) {
                fprintf(stderr,"Invalid range string: \"%s\", unexpected \'%c\'.\n",s,s[i]);
                return -EINVAL;
            }
        return 0;
    } else if(ndelim == 1) {
        for(int i=0;i<string.find(':');++i)
            if(!isdigit(s[i])) {
                fprintf(stderr,"Invalid range string: \"%s\", unexpected \'%c\'.\n",s,s[i]);
                return -EINVAL;
            }
        if(string.substr(string.find(':')+1,string.length()-string.find(':')-1) != "end")
            for(int i=string.find(':')+1;i<string.length();++i)
                if(!isdigit(s[i])) {
                    fprintf(stderr,"Invalid range string: \"%s\", unexpected \'%c\'.\n",s,s[i]);
                    return -EINVAL;
                }
        return 1;
    } else if(ndelim == 2) {
        for(int i=0;i<string.find(':');++i)
            if(!isdigit(s[i])) {
                fprintf(stderr,"Invalid range string: \"%s\", unexpected \'%c\'.\n",s,s[i]);
                return -EINVAL;
            }
        if(!isdigit(s[string.find(':')+1]) && s[string.find(':')+1] != '-') {
                fprintf(stderr,"Invalid range string: \"%s\", unexpected \'%c\'.\n",s,s[string.find(':')+1]);
                return -EINVAL;
        }
        for(int i=string.find(':')+2;i<string.rfind(':')-1;++i)
            if(!isdigit(s[i])) {
                fprintf(stderr,"Invalid range string: \"%s\", unexpected \'%c\'.\n",s,s[i]);
                return -EINVAL;
            }
        if(string.substr(string.rfind(':')+1,string.length()-string.rfind(':')-1) != "end")
            for(int i=string.rfind(':')+1;i<string.length();++i)
                if(!isdigit(s[i])) {
                    fprintf(stderr,"Invalid range string: \"%s\", unexpected \'%c\'.\n",s,s[i]);
                    return -EINVAL;
                }
        return 2;
    } else {
        fprintf(stderr,"Invalid range string: \"%s\", unexpected \'%c\'.\n",s,':');
        return -EINVAL;
    }
}

int getopts(int argc,char *argv[],struct options *options) {
    int c;
    int option_index = 0;
    std::string stringarg;
    struct option long_options[] = {
        {"ascii", 0, NULL, 'a'},
        {"binary", 0, NULL, 'b'},
        {"columns", 1, NULL, 'c'},
        {"info", 0, NULL, 'i'},
        {"rows", 1, NULL, 'r'},
        {"trial", 1, NULL, 't'},
        { 0, 0, 0, 0}
    };

    while(1) {
        c = getopt_long(argc,argv,"abc:ir:t:",long_options,&option_index);

        if(c < 0)
            break;

        int delim;
        switch(c) {
          case 'a':
              options->binary = 0;
              break;
          case 'b':
              options->binary = 1;
              break;
          case 'c':

              delim = validate_range_string(optarg);
              if(delim < 0)
                  exit(delim);

              stringarg = optarg;
              switch(delim) {
                case 0:
                    options->cols_start = options->cols_end = strtol(stringarg.c_str(),NULL,10);
                    options->cols_step = 1;
                    break;

                case 1:
                    options->cols_start = strtol(stringarg.substr(0,stringarg.find(':')).c_str(),NULL,10);
                    if(stringarg.substr(stringarg.rfind(':')+1,stringarg.length()-stringarg.rfind(':')-1) == "end")
                        options->cols_end = -1;
                    else
                        options->cols_end = strtol(stringarg.substr(stringarg.rfind(':')+1,stringarg.length()-stringarg.rfind(':')-1).c_str(),NULL,10);
                    options->cols_step = 1;
                    break;
                    
                case 2:
                    options->cols_start = strtol(stringarg.substr(0,stringarg.find(':')).c_str(),NULL,10);
                    if(stringarg.substr(stringarg.rfind(':')+1,stringarg.length()-stringarg.rfind(':')-1) == "end")
                        options->cols_end = -1;
                    else
                        options->cols_end = strtol(stringarg.substr(stringarg.rfind(':')+1,stringarg.length()-stringarg.rfind(':')-1).c_str(),NULL,10);
                    options->cols_step = strtol(stringarg.substr(stringarg.find(':')+1,stringarg.rfind(':')-stringarg.find(':')-1).c_str(),NULL,10);
              }
              break;
          case 'i':
              options->info = 1;
              break;
          case 'r':

              delim = validate_range_string(optarg);
              if(delim < 0)
                  exit(delim);

              stringarg = optarg;
              switch(delim) {
                case 0:
                    options->rows_start = options->rows_end = strtol(stringarg.c_str(),NULL,10);
                    options->rows_step = 1;
                    break;

                case 1:
                    options->rows_start = strtol(stringarg.substr(0,stringarg.find(':')).c_str(),NULL,10);
                    if(stringarg.substr(stringarg.rfind(':')+1,stringarg.length()-stringarg.rfind(':')-1) == "end")
                        options->rows_end = -1;
                    else
                        options->rows_end = strtol(stringarg.substr(stringarg.rfind(':')+1,stringarg.length()-stringarg.rfind(':')-1).c_str(),NULL,10);
                    options->rows_step = 1;
                    break;
                    
                case 2:
                    options->rows_start = strtol(stringarg.substr(0,stringarg.find(':')).c_str(),NULL,10);
                    if(stringarg.substr(stringarg.rfind(':')+1,stringarg.length()-stringarg.rfind(':')-1) == "end")
                        options->rows_end = -1;
                    else
                        options->rows_end = strtol(stringarg.substr(stringarg.rfind(':')+1,stringarg.length()-stringarg.rfind(':')-1).c_str(),NULL,10);
                    options->rows_step = strtol(stringarg.substr(stringarg.find(':')+1,stringarg.rfind(':')-stringarg.find(':')-1).c_str(),NULL,10);
              }
              break;
          case 't':
              options->trial = strtol(optarg,NULL,10);
              break;
        };

    };

    if(optind == argc) {
        fprintf(stderr,"Usage: %s [options] <filename>\n",argv[0]);
        fprintf(stderr,"\tMust specify a filename\n");
        exit(-EINVAL);
    } else if(optind == argc-1) {
        options->filename = argv[optind];
    } else {
        fprintf(stderr,"Usage: %s [options] <filename>\n",argv[0]);
        fprintf(stderr,"\tToo many arguments\n");
        exit(-EINVAL);
    }
}

void print_trial_info(hid_t trial,int num_trial) {

    char string_data[512];
    hid_t string_type = H5Tcopy(H5T_C_S1);
    size_t string_size = 512;
    H5Tset_size(string_type,string_size);

    printf("Trial #%d\n",num_trial);

    hid_t data = H5Dopen(trial,"Date",H5P_DEFAULT);
    H5Dread(data,string_type,H5S_ALL,H5S_ALL,H5P_DEFAULT,string_data);
    H5Dclose(data);

    printf("\tDate: %s\n",string_data);

    data = H5Dopen(trial,"Synchronous Data/Channel Data",H5P_DEFAULT);
    hsize_t nchans = H5Tget_size(H5Dget_type(data))/sizeof(double);
    H5Dclose(data);

    data = H5PTopen(trial,"Synchronous Data/Channel Data");
    hsize_t nsamples;
    H5PTget_num_packets(data,&nsamples);

    printf("\t%llu Channels X %llu samples\n",nchans,nsamples);

    for(int i=1;i<=nchans;++i) {
        std::stringstream channel_name;
        channel_name << "Synchronous Data/Channel "  << i << " Name";
        data = H5Dopen(trial,channel_name.str().c_str(),H5P_DEFAULT);
        H5Dread(data,string_type,H5S_ALL,H5S_ALL,H5P_DEFAULT,string_data);
        H5Dclose(data);

        printf("\t\t#%d: %s\n",i,string_data);
    }

    printf("\n");
}
