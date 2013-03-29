function example_uisplitpane
% Illustrates how to use uisplitpane.
%
% See also: uisplitpane, uisplitter

% Copyright 2010 Levente Hunyadi, http://hunyadi.info.hu

figure( ...
    'Name', 'uisplitpane demo');
x = 0:100;
[left,right] = uisplitpane('Orientation','hor');
[lleft,lright] = uisplitpane(left,'Orientation','hor');
[top,bottom] = uisplitpane(lright,'Orientation','vert');
[rtop,rbottom] = uisplitpane(right,'Orientation','vert');