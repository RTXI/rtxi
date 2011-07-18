function [fileinfo] = rtxi_read(fname)
% [fileinfo] = rtxi_read(fname)
% 
% This function gets basic information the HDF5 file and its structure.
%
% AUTHOR: Risa Lin
% DATE:  10/31/2010

fileinfo = hdf5info(fname); % load default information from built-in MATLAB function
% add additional useful data to the structure
fileinfo.numTrials = size(fileinfo.GroupHierarchy.Groups,2);