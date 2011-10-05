function [parameters,numParameters] = printParameters(fname,trialNum)
% [parameters,numParameters] = printParameters(fname,trialNum)
%
% This function returns all the parameters used in the specified trial of
% an RTXI HDF5 file and also prints them to the screen.
%
% AUTHOR: Risa Lin
% DATE:  10/31/2010
%
% MODIFIED:
% 9/27/2011 - now supports string parameters

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

for (p=1:numParameters);
  if (ischar(parameters.values{p})) % print string
    s = sprintf('parameter(%i): %s : %s : %s\n',p,parameters.modelName{p},parameters.parameterName{p},parameters.values{p});
    disp(s)
  else % print the second value, the first is the timestamp
    s = sprintf('parameter(%i): %s : %s : %f\n',p,parameters.modelName{p},parameters.parameterName{p},parameters.values{p}{1,2});
    disp(s)
  end
end
