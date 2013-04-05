%% get basic file info
clear; clc

fname = 'testGwaveform.h5';

fileinfo = rtxi_read(fname);
trial1 = getTrial(fname,1);
% trial2 = getTrial(fname,2);
% trial3 = getTrial(fname,3);
%printParameters(fname,1);


%% browse a HDF5 file starting with a particular trial
clc
close all
rtxifig = rtxibrowse(fname,1);
% rtxifig2 = rtxibrowse('new',fname,2);

%% browse a HDF5 file starting with a particular trial, reusing a figure
% handle

rtxifig = rtxibrowse(rtxifig,fname,2);


%% add some data to the hdf5 file

file = H5F.open(fname,'H5F_ACC_RDWR', 'H5P_DEFAULT');

% add a dataset
dset = single(rand(10,10));
dset_details.Location = '/Trial1';
dset_details.Name = 'Random';


hdf5write(fname, dset_details, dset, 'WriteMode', 'append')

% get the dataset back out
hdf5read(fname,'/Trial1/Random')

H5F.close(file);

