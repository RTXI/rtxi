function [panel1,panel2,splitter] = uisplitpane(varargin)
% Splits a container into two resizable sub-containers (panes).
%
% This function resembles UISplitPane by Yair Altman but uses fully
% documented MatLab functionality only. Usage is mostly compatible, and
% this wrapper function is available to exhibit a similar interface as
% UISplitPane.
%
% Syntax:
%    uisplitpane(parent, key, value, key, value, ...)
%    uisplitpane(key, value, key, value, ...)
%
% Input arguments:
% parent:
%    a graphics handle to a figure or uipanel
% key, value:
%    key-value pairs to pass as constructor arguments to uisplitter
%
% Output arguments:
% panel1:
%    left or bottom sub-container handle, depending on orientation
% panel2:
%    right or top sub-container handle, depending on orientation
% splitter:
%    a uisplitter instance
%
% Examples:
% [left,right] = uisplitpane;
%    creates a uisplitpane with default horizontal orientation
% [bottom,top] = uisplitpane('Orientation','vertical');
%    creates a uisplitpane with vertical orientation
% [bottom,top] = uisplitpane(fig,'Orientation','ver');
%    creates a uisplitpane explicitly specifying the parent container
% [left,right] = uisplitpane('DividerLocation',0.25);
%    creates a pane where the left part is one third of the right
%
% References:
% Yair Altman, UISplitPane, http://undocumentedmatlab.com/blog/uisplitpane/
%
% See also: uisplitter

% Copyright 2010 Levente Hunyadi, http://hunyadi.info.hu

splitter = uisplitter(varargin{:});
panel1 = splitter.LeftOrBottomPaneHandle;
panel2 = splitter.RightOrTopPaneHandle;
