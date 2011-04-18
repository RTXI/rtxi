#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sstream>

#include <hdf5.h>
#include <hdf5_hl.h>

int main(int argc,char *argv[]) {

    // hide HDF5 stack tracks on all error returns
    H5Eset_auto2(H5E_DEFAULT,NULL,NULL);

    if(argc != 2) {
        fprintf(stderr,"Usage: %s <filename>\n",argv[0]);
        return -EINVAL;
    }

    char cmdBuff[200];
    sprintf(cmdBuff,"cp %s %s.old",argv[1],argv[1]);
    system(cmdBuff);

    hid_t fid = H5Fopen(argv[1],H5F_ACC_RDWR,H5P_DEFAULT);
    if(fid < 0) {
        fprintf(stderr,"Failed to open %s.\n",argv[0]);
        return fid;
    }

    // create a temporary file to store the large amount of stream data
    char tempfile_template[] = "rtxi_hdf_matlabize.XXXXXX";
    int tmpfd = mkstemp(tempfile_template);
    if(tmpfd < 0) {
        fprintf(stderr,"Failed to create a temporary file for buffering data.\n");
        return -errno;
    }
    unlink(tempfile_template);

    int trial_num = 1;
    std::stringstream trial_name;
    trial_name << "/Trial1";
    H5G_info_t junk;

    while(H5Gget_info_by_name(fid,trial_name.str().c_str(),&junk,H5P_DEFAULT) >= 0) {

        // determine the dimension of the data
        trial_name << "/Syncronous Data/Channel Data";
        hid_t table = H5Dopen(fid,trial_name.str().c_str(),H5P_DEFAULT);
        if(table >= 0) {

            hid_t type = H5Dget_type(table);
            if(H5Tequal(type,H5T_IEEE_F64LE)) {
                H5Dclose(type);
                H5Dclose(table);


                ++trial_num;
                trial_name.str("");
                trial_name << "/Trial" << trial_num;
                continue;
            }

            hsize_t cols = H5Tget_size(type)/sizeof(double);
            H5Dclose(type);
            H5Dclose(table);

            hsize_t rows;
            table = H5PTopen(fid,trial_name.str().c_str());
            H5PTget_num_packets(table,&rows);

            lseek(tmpfd,SEEK_SET,0);
            for(int i=0;i<rows;++i) {
                double data[cols];

                H5PTget_next(table,1,data);
                write(tmpfd,data,sizeof(data));
            }

            H5PTclose(table);
            H5Ldelete(fid,trial_name.str().c_str(),H5P_DEFAULT);

            hsize_t dims[] = { rows, cols };
            hid_t space = H5Screate_simple(2,dims,dims);
            table = H5Dcreate(fid,trial_name.str().c_str(),H5T_IEEE_F64LE,space,H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
            H5Sclose(space);

            lseek(tmpfd,SEEK_SET,0);
            {

                hid_t data_space = H5Screate_simple(1,&cols,&cols);
                H5Sselect_all(data_space);

                double data[cols];
                hsize_t start[2] = { 0, 0 };
                hsize_t count[2] = { 1, cols};

                hid_t select = H5Dget_space(table);

                for(int i=0;i<rows;++i) {

                    read(tmpfd,data,sizeof(data));

                    start[0] = i;
                    H5Sselect_hyperslab(select,H5S_SELECT_SET,start,NULL,count,NULL);

                    H5Dwrite(table,H5T_IEEE_F64LE,data_space,select,H5P_DEFAULT,data);
                }
                H5Sclose(select);
                H5Sclose(data_space);
            }

            H5Dclose(table);
        }

        ++trial_num;
        trial_name.str("");
        trial_name << "/Trial" << trial_num;
    }

    H5Fclose(fid);
    close(tmpfd);

    return 0;

}
