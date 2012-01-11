function rtxifig = rtxibrowse(rtxifig,fname,trialNum)
% rtxifig = rtxibrowse(rtxifig,fname,trialNum)
%
% INPUTS:
%   rtxifig - figure handle
%   fname   - filename of HDF5 file you want to read from
%   trialNum = trial number from HDF5 file that you want to plot
%
%
% OUTPUTS:
%   rtxifig - returns the figure handle that was used
%
% This function calls rtxiplot.m and creates an interface that allows you
% to plot two different channels from the same trial of an RTXI HDF5 File.
% Each axes is linked to its own listbox which contains all the available
% channels. The time axis is automatically computed from the experimental
% time step extracted from the HDF5 file. Double-clicking on a channel in
% the listbox will replot that channel in a separate window. The interface 
% also lists all the parameters settings for each trial. The figure name
% tells you what file the trial was plotted from and which trial you are
% plotting.
%
% AUTHOR: Risa Lin
% DATE:   10/31/2010

if (isscalar(rtxifig) && ishandle(rtxifig))  % re-use existing figure, needs 3 arguments
    if nargin == 3
        tag = get(rtxifig, 'Tag');
        assert(~isempty(tag) && strcmp(tag, '__rtxiplot__'), ...  % verify figure tag
            'rtxiplot:ArgumentTypeMismatch', 'An rtxiplot figure is expected.');
    else
        error('You must have 3 arguments if you are specifying a figure handle: (rtxifig, filename,trialnumber)');
    end
else
    if nargin ==2
        trialNum = fname;
        fname = rtxifig;
        end
    rtxifig = rtxiplot('new');
    scrsz = get(0,'ScreenSize');
    % scrsz [left, bottom, width, height]
    set(rtxifig,'Position',[100 scrsz(4)/2 900 scrsz(4)/2]);    
end

fileinfo = rtxi_read(fname);
figname = ['RTXI HDF5 Browser v0.1 | ',fname,' | Trial ',num2str(trialNum),'/',num2str(fileinfo.numTrials)];
set(rtxifig,'Name',figname);

trial = getTrial(fname,trialNum);

% Trial info:
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

% This metadata is set as application data attached the figure.

meta.datetime = trial.datetime;
meta.fname = trial.file;
meta.numTrials = fileinfo.numTrials;
meta.trialNum = trialNum;

for i=1:trial.numChannels
    rtxiplot(rtxifig,trial.channels{i}, trial.parameters, meta, trial.time, trial.data(:,i), 'k-');
end
