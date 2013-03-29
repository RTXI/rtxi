function fig = rtxiplot(varargin)
% fig = rtxiplot(varargin)
%
% INPUTS: varargin must contain a minimum of three arguments and can use 4
% optional arguments in the following order:
%
% optional: figure handle
%           captions for listbox (cell)
%           parameters (a struct: entries are listed in the parameter panel
%                      of the GUI)
%           meta (a struct: metadata to pass through to the figure)
% required: x data
%           y data
%           plot style ('k-')
%
% OUTPUTS:
%   fig - returns the figure handle that was used
%
% Used by rtxibrowse.m to create the GUI and plot all channels from a
% single trial extracted from an RTXI HDF5 file. The "Prev" and "Next"
% buttons are only applicable in this scenario since it uses metadata such
% as the HDF5 file name to browse through all the trials.
%
% AUTHOR: Risa Lin
% DATE:   10/31/2010
%
% MODIFIED:
% 9/27/2011 - supports unmatlabized RTXI files and strings

k = 1;  % parameter counter
if k <= numel(varargin) && isscalar(varargin{k}) && ishandle(varargin{k})  % re-use existing figure
  fig = varargin{k};
  tag = get(fig, 'Tag');
  assert(~isempty(tag) && strcmp(tag, '__rtxiplot__'), ...  % verify figure tag
    'rtxiplot:ArgumentTypeMismatch', 'An rtxiplot figure is expected.');
  k = k + 1;
elseif k <= numel(varargin) && ischar(varargin{k}) && strcmp('new', varargin{k})  % force creation of new figure
  fig = rtxiplot_figure();
  k = k + 1;
else
  curfig = get(0,'CurrentFigure');
  h = findobj('Type', 'figure', 'Tag', '__rtxiplot__');
  if isempty(h)  % create new figure if no rtxiplot figure exists yet
    fig = rtxiplot_figure();
    setappdata(fig,'reset',0); % do not reset listbox contents
  elseif isscalar(h)  % a single rtxiplot figure exists
    if ~isempty(curfig) && h == curfig  % currently active figure is an rtxiplot figure
      fig = curfig;
    else
      fig = h;
      figure(fig);  % currently active figure is not an rtxiplot figure, activate the only rtxiplot figure
    end
  else  % multiple rtxiplot figures exist
    if ~isempty(curfig)
      ix = find(h == curfig);  % ix is either empty or a scalar
    else
      ix = [];
    end
    if isscalar(ix)  % current figure is an rtxiplot figure
      fig = curfig;
    else  % isempty(ix)
      fig = h(1);  % use rtxiplot figure topmost in stacking order
      figure(fig);
      warning('rtxiplot:InvalidOperation', ...
        'Currently active figure is not an rtxiplot figure, the topmost rtxiplot figure has been activated.');
    end
  end
end

if k <= numel(varargin) && ischar(varargin{k})  % get channel names as captions for listbox
  caption = varargin{k};
  k = k + 1;
else  % use default caption
  caption = [];
end

if k <= numel(varargin) && isstruct(varargin{k})  % get parameters
  parameters = varargin{k};
  k = k + 1;
else  % use default caption
  parameters = [];
end

if k <= numel(varargin) && isstruct(varargin{k})  % get meta data
  meta = varargin{k};
  k = k + 1;
else  % use default caption
  meta = [];
end
setappdata(fig,'meta',meta)


if k > numel(varargin)  % nothing to plot
  if ~isempty(caption)
    warning('rtxiplot:InvalidOperation', ...
      'A caption is specified but no data to plot has been passed.');
    return;
  end
  return;
end
varargin = varargin(k:end);  % drop preprocessed arguments

ppanel = findobj(fig,'Tag','parameterpanel');
uicontrol(ppanel, ...
  'Style','text', ...
  'Units','normalized', ...
  'Position', [0 .9 1 0.1], ...
  'String','INITIAL PARAMETERS');

if (ischar(parameters.values{1})) % is string
  pstr = ['[{''',parameters.modelName{1},' : ',parameters.parameterName{1},' : ',parameters.values{1}];
else % get the second value, the first is the timestamp
  pstr = ['[{''',parameters.modelName{1},' : ',parameters.parameterName{1},' : ',num2str(parameters.values{1}{1,2})];
end

%pstr = ['[{''',parameters.modelName{1},' : ',parameters.parameterName{1},' : ',num2str(parameters.values{1}{1,2})];
if size(parameters.values{1},1)>1
  pstr=[pstr,'*''}'];
else
  pstr=[pstr,'''}'];
end
for p=2:length(parameters.parameterName)
  if (ischar(parameters.values{p})) % get string
    pstr = [pstr,';{''',strrep(parameters.modelName{p},'''',''''''),' : ',strrep(parameters.parameterName{p},'''',''''''),' : ',parameters.values{p}];
  else % get the second value, the first is the timestamp
    pstr = [pstr,';{''',strrep(parameters.modelName{p},'''',''''''),' : ',strrep(parameters.parameterName{p},'''',''''''),' : ',num2str(parameters.values{p}{1,2})];
  end
  
  %pstr = [pstr,';{''',strrep(parameters.modelName{p},'''',''''''),' : ',strrep(parameters.parameterName{p},'''',''''''),' : ',num2str(parameters.values{p}{1,2})];
  if size(parameters.values{p},1)>1
    pstr=[pstr,'*''}'];
  else
    pstr=[pstr,'''}'];
  end
end
pstr = [pstr,']'];

eval(['uicontrol(ppanel,''Style'',''text'',''Units'',''normalized'',''FontSize'',9,''Position'', [.03 0 0.95 0.95],''HorizontalAlignment'',''Left'',''String'',',pstr,');']);

listbox1 = findobj(fig, 'Type', 'uicontrol', 'Tag', 'listbox1');
string = get(listbox1, 'String'); % cell array of all the item strings
if getappdata(fig,'reset')==1;
  userdata = {};
  string = {};
  set(listbox1,'Value',1);
else
  userdata = get(listbox1, 'UserData');
end
if numel(string) > 100
  warning('rtxiplot:InvalidOperation', 'There are too many plots, new plot has not been added.');
  return;
end
if numel(string) == 1 && strcmp(string, '[empty]')  % there is only the placeholder item in the list
  string = {};  % clear placeholder item
  userdata = {};
end
userdata{numel(userdata)+1} = varargin;  % add arguments to plot function
if isempty(caption)
  caption = sprintf('Plot %d', numel(userdata));
end
string{numel(string)+1} = caption;  % add caption used in list box
set(listbox1, ...
  'String', string, ...
  'TooltipString', 'Double-click item (or hit ENTER) to plot in new figure window.', ...
  'UserData', userdata);
if numel(string) <= 1
  rtxiplot_onselectionchange(listbox1);  % show first selection by default
end

listbox2 = findobj(fig, 'Type', 'uicontrol', 'Tag', 'listbox2');
string = get(listbox2, 'String');
if getappdata(fig,'reset')==1;
  userdata = {};
  string = {};
  set(listbox2,'Value',1);
else
  userdata = get(listbox2, 'UserData');
end
if numel(string) > 100
  warning('rtxiplot:InvalidOperation', 'There are too many plots, new plot has not been added.');
  return;
end
if numel(string) == 1 && strcmp(string, '[empty]')  % there is only the placeholder item in the list
  string = {};  % clear placeholder item
  userdata = {};
end
userdata{numel(userdata)+1} = varargin;  % add arguments to plot function
if isempty(caption)
  caption = sprintf('Plot %d', numel(userdata));
end
string{numel(string)+1} = caption;  % add caption used in list box
set(listbox2, ...
  'String', string, ...
  'TooltipString', 'Double-click item (or hit ENTER) to plot in new figure window.', ...
  'UserData', userdata);
if numel(string) <= 1
  rtxiplot_onselectionchange(listbox2);  % show first selection by default
end
setappdata(fig,'reset',0);

function fig = rtxiplot_figure()
% Creates a new rtxiplot figure.

fig = figure( ...
  'Tag', '__rtxiplot__');  % ensures that all necessary controls are present

try
  % try to use uisplitter if available
  
  [leftpanel,rightpanel] = uisplitpane(fig,'DividerWidth', 2,'DividerLocation', 0.25,'Orientation','hor');
  [boxpanel,plotpanel] = uisplitpane(rightpanel,'DividerWidth',2,'DividerLocation',0.25,'Orientation','hor');
  [topbox,bottombox] = uisplitpane(boxpanel,'DividerWidth',2,'DividerLocation',0.5,'Orientation','vert');
  [topplot,bottomplot] = uisplitpane(plotpanel, 'DividerWidth',2,'Orientation','vert');
  [bttnpanel,ppanel] = uisplitpane(leftpanel,'DividerWidth',2,'DividerLocation',0.1,'Orientation','vert');
  set(leftpanel,'Tag','leftpanel');
  set(ppanel,'Tag','parameterpanel');
  set(bttnpanel,'Tag','buttonpanel');
  set(rightpanel,'Tag','rightpanel');
  set(boxpanel,'Tag','leftpanel');
  set(plotpanel,'Tag','plotpanel');
  set(topbox,'Tag','topbox');
  set(bottombox,'tag','bottombox');
  set(bottomplot,'Tag','bottomplot');
  set(topplot,'Tag','topplot');
catch
  msgbox('Get uisplitter!');
end

prev_button = uicontrol(bttnpanel,'Style','pushbutton',...
  'Units','normalized',...
  'String','Prev',...
  'Position',[0.03 0.15 0.4 0.8], ...
  'Callback', @prev_button_callback);

next_button = uicontrol(bttnpanel,'Style','pushbutton',...
  'Units','normalized',...
  'String','Next',...
  'Position',[0.5 0.15 0.4 0.8], ...
  'Callback', @next_button_callback);

listbox1 = uicontrol(topbox, ...  % allows selection of data to plot
  'Style', 'listbox', ...
  'Units', 'normalized', ...
  'Position', [.03 0.03 0.95 .95], ...
  'String', {'[empty]'}, ...  % placeholder item
  'Tag', 'listbox1', ...
  'Callback', @rtxiplot_onselectionchange);

listbox2 = uicontrol(bottombox, ...  % allows selection of data to plot
  'Style', 'listbox', ...
  'Units', 'normalized', ...
  'Position', [0.03 0.03 0.95 .95], ...
  'String', {'[empty]'}, ...  % placeholder item
  'Tag', 'listbox2', ...
  'Callback', @rtxiplot_onselectionchange);

axes('Parent', topplot);
axes('Parent', bottomplot);
set(listbox2, 'UserData', {{}});  % no plot arguments belong to placeholder item
set(listbox1, 'UserData', {{}});  % no plot arguments belong to placeholder item


function rtxiplot_plot(ax, varargin)
% Plots data in an rtxiplot figure with the specified plot arguments.
%
% Input arguments:
% ax:
%    an axes handle graphics object
% varargin (optional):
%    parameters to pass to the plot function

if nargin >= 2
  try
    plot(ax, varargin{:});
    xlabel(ax,'time (s)');
  catch me
    disp('plot function produces an error when passed parameters:');
    args = varargin(:);
    disp(args);
    rtxiplot_error(ax);
    rethrow(me);
  end
else  % nothing to plot
  rtxiplot_hideaxes(ax);
end

function prev_button_callback(src,eventdata)
fig = ancestor(src, 'figure');
meta = getappdata(fig,'meta');
if meta.trialNum-1 >= 1
  trial = getTrial(meta.fname,meta.trialNum-1);
  meta.trialNum = meta.trialNum - 1;
  setappdata(fig,'meta',meta);
  setappdata(fig,'reset',1); % clear listbox contents
  rtxibrowse(fig,meta.fname,meta.trialNum);
else
  msgbox('This is the first trial in the file!');
  
end

function next_button_callback(src,eventdata)
fig = ancestor(src, 'figure');
meta = getappdata(fig,'meta');
if meta.trialNum+1 <= meta.numTrials
  trial = getTrial(meta.fname,meta.trialNum+1);
  meta.trialNum = meta.trialNum + 1;
  setappdata(fig,'meta',meta);
  setappdata(fig,'reset',1); % clear listbox contents
  rtxibrowse(fig,meta.fname,meta.trialNum);
else
  msgbox('This is the last trial in the file!');
  
end


function rtxiplot_onselectionchange(listbox, event) %#ok<INUSD>
% Fired when the selected list item changes.

fig = ancestor(listbox, 'figure');
index = get(listbox, 'Value');
tag = get(listbox,'Tag');
if isempty(index)
  ax = findobj(fig, 'Type', 'axes');
  rtxiplot_hideaxes(ax);
  return;
end

userdata = get(listbox, 'UserData');
args = userdata{index};
allaxes = findobj(fig,'Type','axes');
panels = findobj(fig,'Type','uipanel');
for i=length(panels):-1:1
  if strcmp(get(panels(i),'Tag'),'topplot')
    topaxesparent = panels(i);
  elseif strcmp(get(panels(i),'Tag'),'bottomplot')
    bottomaxesparent = panels(i);
  end
end

switch get(fig, 'SelectionType')
  case 'open' %% when you double-click, copies plot to new figure
    string = get(listbox, 'String');
    caption = string{index};
    if ~isempty(caption)
      meta = getappdata(fig,'meta');
      newfigname = [meta.fname,' | Trial ',num2str(meta.trialNum),'/',num2str(meta.numTrials),' | ',caption];
      newfig = figure('Name', newfigname);  % open new figure window
    else
      newfig = figure;
    end
    ax = axes('Parent', newfig);
    
  otherwise  % pass clean axes to user-defined function
    if strcmp(tag,'listbox1') % find axes handle for top plot
      if get(allaxes(1),'Parent')==topaxesparent
        ax = allaxes(1);
      elseif get(allaxes(2),'Parent')==topaxesparent
        ax = allaxes(2);
      end
    elseif strcmp(tag,'listbox2') % find axes handle for bottom plot
      if get(allaxes(1),'Parent')==bottomaxesparent
        ax = allaxes(1);
      elseif get(allaxes(2),'Parent')==bottomaxesparent
        ax = allaxes(2);
      end
    end
end

if numel(args) > 0 && isa(args{1}, 'function_handle')
  try
    fun = args{1};  % unwrap function handle from cell array
    fun(ax, args{2:end});  % call user-defined function passing axes as argument
  catch me
    rtxiplot_error(ax);
    rethrow(me);
  end
else
  rtxiplot_plot(ax, args{:});
end
set(ax, 'Visible', 'on');

function rtxiplot_hideaxes(ax)

cla(ax, 'reset');
set(ax, 'Visible', 'off');

function rtxiplot_error(ax)

rtxiplot_hideaxes(ax);
msgbox('Error while plotting data to axes, see console for details.', ...
  'Plot error', 'error');
