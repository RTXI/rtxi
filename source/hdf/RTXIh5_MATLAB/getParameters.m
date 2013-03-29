function [parameter,numParameters] = getParameters(fname,trialNum)
% [parameter,numParameters] = getParameters(fname,trialNum)
% 
% This function returns all the parameters used in the specified trial of 
% an RTXI HDF5 file.
%
% AUTHOR: Risa Lin
% DATE:  10/31/2010

if nargin < 2
    if (hasfield(fname,'parameters'))
        printParameters(fname);
        return
    else
        trialNum = 1;
    end
end

fileinfo = rtxi_read(fname);

if trialNum>fileinfo.numTrials
    error('There are only %i trials in this file.\n',fileinfo.numTrials)
end

numParameters = size(fileinfo.GroupHierarchy(1).Groups(trialNum).Groups(2).Datasets,2);
parameter.parameterName = cell(numParameters,1);
parameter.values = cell(numParameters,1);
parameter.modelName = cell(numParameters,1);

eval(['ds = double(hdf5read(fname,''Trial',num2str(trialNum),'/Downsampling Rate''));'])
eval(['trial.exp_dt = double(hdf5read(fname,''Trial',num2str(trialNum),'/Period (ns)''))*1e-9;'])
trial.data_dt = trial.exp_dt*ds;

for (p=1:numParameters);    
    fullName = fileinfo.GroupHierarchy(1).Groups(trialNum).Groups(2).Datasets(p).Name;
    nameStart = findstr(fullName, '/');
    nameEnd = findstr(fullName,':');
    
    dset = hdf5read(fileinfo.GroupHierarchy(1).Groups(trialNum).Groups(2).Datasets(p));
    
    parameter.modelName{p} = fullName(nameStart(end)+1:nameEnd-2);
    parameter.parameterName{p} = fullName(nameEnd+2:end);
    
    % test if it's a comment by testing for string type
     
    if (strcmp(fileinfo.GroupHierarchy(1).Groups(trialNum).Groups(2).Datasets(p).Datatype.Class,'H5T_STRING'))
        parameter.values{p} = dset(1).Data;
        for i=2:size(dset,1)
            parameter.values{p} = [parameter.values{p} dset(i).Data];
        end
    else
        % test if there have been changes to the parameter
        parameter.values{p} = dset(1).Data;
        parameter.values{p}{1,1} = double(parameter.values{p}{1,1})*trial.data_dt;
        for i=2:size(dset,1)
            parameter.values{p} = [parameter.values{p};dset(i).Data];
            parameter.values{p}{i,1} = double(parameter.values{p}{i,1})*trial.data_dt; % convert time of parameter change to s
        end
    end
    
end

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
