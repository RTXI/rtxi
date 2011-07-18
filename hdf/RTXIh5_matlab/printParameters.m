function [parameters,numParameters] = printParameters(fname,trialNum)
% [parameters,numParameters] = printParameters(fname,trialNum)
% 
% This function returns all the parameters used in the specified trial of 
% an RTXI HDF5 file and also prints them to the screen.
%
% AUTHOR: Risa Lin
% DATE:  10/31/2010

if nargin < 2
    if (hasfield(fname,'parameters'))
        for (p=1:fname.numParameters);
            s = sprintf('parameter(%i): %s : %s : %f\n',p,fname.parameters.modelName{p},fname.parameters.parameterName{p},fname.parameters.values{p});
            disp(s)
        end
        return
    else
        trialNum = 1;
    end
end

fileinfo = rtxi_read(fname);
[parameters,numParameters] = getParameters(fname,trialNum);

for (p=1:15);
    s = sprintf('parameter(%i): %s : %s : %f\n',p,parameters.modelName{p},parameters.parameterName{p},parameters.values{p});
    disp(s)
end
