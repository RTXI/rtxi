function [trial] = getTrial(fname,trialNum)
% [trial] = getTrial(fname,trialNum)
% 
% This function returns all the channel data, metadata, and parameters used 
% in the specified trial of an RTXI HDF5 file.
%
%        parameters: [1x1 struct]
%     numParameters: 15
%          datetime: '2009-10-21T23:02:28'
%            exp_dt: 1.0000e-04
%           data_dt: 1.0000e-04
%         timestart: '10:01:29.6065'
%          timestop: '10:01:50.629'
%            length: 21.0225
%       numChannels: 3
%          channels: {1x3 cell}
%              data: [78813x3 double]
%              time: [78813x1 double]
%              file: 'dclamp.h5'
%
% AUTHOR: Risa Lin
% DATE:  10/31/2010

% MODIFIED:
% 11/11/2010 - corrected extraction of synchronous channel names when >=10
% channels are saved


if nargin < 2
    trialNum = 1;
end

fileinfo = rtxi_read(fname);
if trialNum>fileinfo.numTrials
    error('There are only %i trials in this file.\n',fileinfo.numTrials)
end

[trial.parameters, trial.numParameters] = getParameters(fname,trialNum);

eval(['dset = hdf5read(fname,''Trial',num2str(trialNum),'/Date'');'])
trial.datetime = dset.Data;
eval(['ds = double(hdf5read(fname,''Trial',num2str(trialNum),'/Downsampling Rate''));'])
eval(['trial.exp_dt = double(hdf5read(fname,''Trial',num2str(trialNum),'/Period (ns)''))*1e-9;'])
trial.data_dt = trial.exp_dt*ds;
eval(['trial.timestart = double(hdf5read(fname,''Trial',num2str(trialNum),'/Timestamp Start (ns)''))*1e-9;'])
eval(['trial.timestop = double(hdf5read(fname,''Trial',num2str(trialNum),'/Timestamp Stop (ns)''))*1e-9;'])

trial.timestart = convertTime(trial.timestart);
trial.timestop = convertTime(trial.timestop);

% Old Data Recorder
% trial.numChannels = size(fileinfo.GroupHierarchy(1).Groups(trialNum).Groups(3).Datasets,2)-1;
% 
% for i=1:trial.numChannels
%     dset = hdf5read(fileinfo.GroupHierarchy(1).Groups(trialNum).Groups(3).Datasets(i));
%     trial.channels{i} = dset.Data;
% end
% 
% data = hdf5read(fileinfo.GroupHierarchy(1).Groups(trialNum).Groups(3).Datasets(trial.numChannels+1));
% numsamples = size(data,2)/trial.numChannels;
% trial.data = reshape(data,trial.numChannels,numsamples);
% trial.data = trial.data';

% New Data Recorder
trial.numChannels = fileinfo.GroupHierarchy(1).Groups(trialNum).Groups(3).Datasets(end).Dims(1);
trial.data = hdf5read(fileinfo.GroupHierarchy(1).Groups(trialNum).Groups(3).Datasets(end));
trial.data = trial.data';

for i=1:trial.numChannels
    s = fileinfo.GroupHierarchy.Groups(trialNum).Groups(3).Datasets(i).Name;
    dset = hdf5read(fileinfo.GroupHierarchy(1).Groups(trialNum).Groups(3).Datasets(i));
    trial.channels{str2double(s(findstr(s,'Channel')+8:end-5))} = dset.Data;
end
numsamples = fileinfo.GroupHierarchy(1).Groups(trialNum).Groups(3).Datasets(end).Dims(2);
trial.time = 0:trial.data_dt:numsamples*trial.data_dt-trial.data_dt;
trial.time = trial.time';
trial.length = trial.time(end);

trial.file = fname;

end

function strtime = convertTime(time)
hour = time/3600;
minute = rem(hour,1)*60;
second = rem(minute,1)*60;
hour = floor(hour);
minute = floor(minute);

if hour < 10; strtime = ['0',num2str(hour)]; else; strtime = num2str(hour);end
if minute < 10; strtime = [strtime,':0',num2str(minute)]; else; strtime = [strtime,':',num2str(minute)];end
if second < 10; strtime = [strtime,':0',num2str(second)]; else; strtime = [strtime,':',num2str(second)];end
end


