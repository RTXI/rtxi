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
        if (size(dset,1) > 1)
            changes = 1;
        end
        parameter.values{p} = dset(1).Data{2};
    end
    
end

end

