classdef uisplitter < hgsetget
% Splits a container into two resizable sub-containers (panes).
%
% uisplitter divides a container into two sub-containers with a movable
% splitter. The container to split can be either a figure or a uipanel with
% no children. Depending on orientation, the splitter is either a vertical
% bar that separates a left and right pane (for horizontal orientation), or
% a horizontal bar that separates a bottom and top pane (for vertical
% orientation). If no container is specified, the figure returned by gcf is
% assumed as the parent container. Once the container is split,
% sub-containers may be freely populated with MatLab components. The
% relative size of the sub-containers may be adjusted interactively using
% the mouse, or programmatically from code.
%
% Settings:
% * Orientation: Split pane orientation 'horizontal' or 'vertical'; a
%   horizontally-oriented split pane consists of left and right part, a
%   vertically-oriented consists of top and bottom part.
% * DividerLocation: Divider position in normalized units, 0 corresponds
%   to far left or bottom, 1 to far right or top, depending on orientation.
% * DividerWidth: Divider width in pixels.
% * DividerColor: Divider background color.
%
% See also: uisplitpane, gcf

% Copyright 2010 Levente Hunyadi, http://hunyadi.info.hu

    properties
        DividerLocation = 0.5;     % Divider position in normalized units.
        DividerWidth = 5;          % Divider width in pixels.
        DividerColor = [0,0,0];    % Divider background color.
        %DividerMinLocation = 0.1;
        %DividerMaxLocation = 0.9;
    end
    properties (SetAccess = private)
        Orientation = 'horizontal';  % Split pane orientation ('horizontal' or 'vertical').
        Parent;
        
        LeftOrBottomPaneHandle;
        RightOrTopPaneHandle;
        DividerHandle;
    end
    methods
        function self = uisplitter(varargin)
        % Construct a uisplitter instance from a variable argument list.
        
            % set parent
            if nargin > 0 && isscalar(varargin{1}) && ishandle(varargin{1})
                self.Parent = varargin{1};
                validatestring(get(self.Parent,'Type'), {'figure','uipanel'}, 'uisplitpane', 'get(Parent,''Type'')');
                varargin = varargin(2:end);
            else
                self.Parent = gcf;
            end
            assert(isempty(get(self.Parent,'Children')), 'uisplitpane:InvalidOperation', ...
                'uisplitpane cannot split nonempty containers.');
            
            % set arguments
            names = fieldnames(self);
            validateattributes(numel(varargin), {'numeric'}, {'even'}, 'uisplitpane', 'argument count');
            for k = 1 : 2 : numel(varargin)
                validateattributes(varargin{k}, {'char'}, {'nonempty','row'}, 'uisplitpane', 'argument name');
                name = validatestring(varargin{k}, names, 'uisplitpane', 'argument name');
                self.(name) = varargin{k+1};  % override default as necessary
            end
            
            self.Initialize();
        end
        
        function Initialize(self)
        % Set up a split pane.

            % create container panel for protecting ResizeFcn
            container = uipanel(self.Parent, ...
                'Units', 'normalized', ...
                'Position', [0 0 1 1], ...
                'BorderType', 'none', ...
                'Visible', 'off', ...
                'ResizeFcn', @(src,evt) self.OnResize());

            self.LeftOrBottomPaneHandle = uipanel(container, ...
                'Units', 'normalized', ...
                'BorderType', 'none', ...
                'BorderWidth', 0);
            self.RightOrTopPaneHandle = uipanel(container, ...
                'Units', 'normalized', ...
                'BorderType', 'none', ...
                'BorderWidth', 0);
            self.DividerHandle = uipanel(container, ...
                'Units', 'normalized', ...
                'BackgroundColor', self.DividerColor, ...
                'ForegroundColor', [0 0 0], ...
                'BorderType', 'none', ...
                'Tag', '__uisplitter__', ...
                'UserData', self);
            switch self.Orientation
                case 'vertical'
                    % splitter
                    set(self.DividerHandle, ...
                        'Position', [0 self.DividerLocation 1 1]);
                    pos = getpixelposition(self.DividerHandle);
                    setpixelposition(self.DividerHandle, [0,pos(2)-floor(0.5*self.DividerWidth),pos(3),self.DividerWidth]);
                    % bottom panel
                    set(self.LeftOrBottomPaneHandle, ...
                        'Position', [0 0 1 self.DividerLocation]);
                    pos = getpixelposition(self.LeftOrBottomPaneHandle);
                    setpixelposition(self.LeftOrBottomPaneHandle, [0,0,pos(3),pos(4)-floor(0.5*self.DividerWidth)]);
                    % top panel
                    set(self.RightOrTopPaneHandle, ...
                        'Position', [0 self.DividerLocation 1 min(1,1-self.DividerLocation)]);
                    pos = getpixelposition(self.RightOrTopPaneHandle);
                    setpixelposition(self.RightOrTopPaneHandle, [0,pos(2)+ceil(0.5*self.DividerWidth),pos(3),pos(4)-ceil(0.5*self.DividerWidth)]);
                otherwise
                    % splitter
                    set(self.DividerHandle, ...
                        'Position', [self.DividerLocation 0 1 1]);
                    pos = getpixelposition(self.DividerHandle);
                    setpixelposition(self.DividerHandle, [pos(1)-floor(0.5*self.DividerWidth),0,self.DividerWidth,pos(4)]);
                    % left panel
                    set(self.LeftOrBottomPaneHandle, ...
                        'Position', [0 0 self.DividerLocation 1]);
                    pos = getpixelposition(self.LeftOrBottomPaneHandle);
                    setpixelposition(self.LeftOrBottomPaneHandle, [0,0,pos(3)-floor(0.5*self.DividerWidth),pos(4)]);
                    % right panel
                    set(self.RightOrTopPaneHandle, ...
                        'Position', [self.DividerLocation 0 min(1,1-self.DividerLocation) 1]);
                    pos = getpixelposition(self.RightOrTopPaneHandle);
                    setpixelposition(self.RightOrTopPaneHandle, [pos(1)+ceil(0.5*self.DividerWidth),0,pos(3)-ceil(0.5*self.DividerWidth),pos(4)]);
            end
            set(container, 'Visible', 'on');
            uisplitter.SubscribeEvents(ancestor(self.DividerHandle, 'figure'));
        end

        function set.Orientation(self, value)
        % Set divider orientation ('horizontal' or 'vertical').
            validateattributes(value, {'char'}, {'nonempty','row'}, 'uisplitpane', 'Orientation');
            self.Orientation = validatestring(value, {'horizontal','vertical'}, 'uisplitpane', 'Orientation');
        end
        
        function set.DividerLocation(self, value)
        % Set divider location in normalized units.
            validateattributes(value, {'numeric'}, {'real','scalar','>=', 0, '<=', 1}, 'uisplitpane', 'DividerLocation');
            self.DividerLocation = value;
            if ~isempty(self.DividerHandle) %#ok<MCSUP>
                switch self.Orientation %#ok<MCSUP>
                    case 'vertical'
                        set(self.DividerHandle, 'Position', [0 value 1 0.01]); %#ok<MCSUP>
                    otherwise
                        set(self.DividerHandle, 'Position', [value 0 0.01 1]); %#ok<MCSUP>
                end
                pos = getpixelposition(self.DividerHandle); %#ok<MCSUP>
                switch self.Orientation %#ok<MCSUP>
                    case 'vertical'
                        pos(4) = self.DividerWidth; %#ok<MCSUP>
                    otherwise
                        pos(3) = self.DividerWidth; %#ok<MCSUP>
                end
                setpixelposition(self.DividerHandle, pos); %#ok<MCSUP>
                self.OnResize();
            end
        end
        
        function set.DividerWidth(self, value)
        % Set divider width in pixels.
            validateattributes(value, {'numeric'}, {'positive','integer','scalar'}, 'uisplitpane', 'DividerWidth');
            self.DividerWidth = value;
            if ~isempty(self.DividerHandle) %#ok<MCSUP>
                self.OnResize();
            end
        end
        
        function set.DividerColor(self, color)
        % Set divider color as RGB triplet or color name.
            if ischar(color)
                validateattributes(color, {'char'}, {'nonempty','row'});
                switch lower(color)
                    case {'k','black'},    value = [0,0,0];
                    case {'r','red'},      value = [1,0,0];
                    case {'g','green'},    value = [0,1,0];
                    case {'b','blue'},     value = [0,0,1];
                    case {'y','yellow'},   value = [1,1,0];
                    case {'m','magenta'},  value = [1,0,1];
                    case {'c','cyan'},     value = [0,1,1];
                    case {'w','white'},    value = [1,1,1];
                    otherwise,             value = color;
                end
            else
                validateattributes(color, {'numeric'}, {'nonempty','vector'});
                value = color;
            end
            self.DividerColor = value;
            if ~isempty(self.DividerHandle) %#ok<MCSUP>
                set(self.DividerHandle, 'BackgroundColor', value); %#ok<MCSUP>
            end
        end
        
        function Move(self, pnt)
        % Move the splitter to the specified location.
        %
        % Input arguments:
        % pnt:
        %    point relative to the figure coordinate system, e.g. as
        %    returned by get(fig,'CurrentPoint')
            parentpos = getpixelposition(get(self.DividerHandle,'Parent'), true);
            left = min(max(pnt(1)-floor(0.5*self.DividerWidth),5), parentpos(3)-5-ceil(0.5*self.DividerWidth));
            bottom = min(max(pnt(2)-floor(0.5*self.DividerWidth),1), parentpos(4)-1-ceil(0.5*self.DividerWidth));
            pos = getpixelposition(self.DividerHandle, true);
            switch self.Orientation
                case 'vertical'
                    pos(2) = bottom;
                    pos(4) = parentpos(4) - bottom;
                otherwise
                    pos(1) = left;
                    pos(3) = parentpos(3) - left;
            end
            setpixelposition(self.DividerHandle, pos, true);
            self.OnResize();
        end
    end
    methods (Static)
        function splitter = GetActive(fig)
        % The active splitter in the figure.
        % A splitter is active is the mouse pointer is within its boundary
        % rectangle.
            pnt = get(fig, 'CurrentPoint');
            handles = findobj(fig, 'Tag', '__uisplitter__');  % get all splitters in figure
            for k = 1 : numel(handles)
                pos = getpixelposition(handles(k), true);
                if ...  % mouse pointer is over one of the splitters, add 1px tolerance
                        pnt(1) >= pos(1)-1 && pnt(1) <= pos(1)+pos(3)+1 && ...
                        pnt(2) >= pos(2)-1 && pnt(2) <= pos(2)+pos(4)+1
                    splitter = get(handles(k), 'UserData');  % set active splitter
                    return;
                end
            end
            splitter = [];
        end
        
        function SubscribeEvents(fig)
        % Subscribe to mouse motion and mouse button events.

            validateattributes(fig, {'numeric'}, {'real','scalar'});
            validatestring(get(fig,'Type'), {'figure'}, 'uisplitpane', 'get(Parent,''Type'')');
        
            % bind event handles to figure at most once
            uibindonce(fig, 'WindowButtonDownFcn', @uisplitpane_onmousedown);
            uibindonce(fig, 'WindowButtonUpFcn', @uisplitpane_onmouseup);
            uibindonce(fig, 'WindowButtonMotionFcn', @uisplitpane_onmouseover);

            % variables for communication between event handlers
            activesplitter = [];  % currently active splitter
            cursor = [];          % cursor before mouse pointer would enter boundary rectangle of splitter
            draganddrop = false;  % whether a drag-and-drop operation is in progress

            function uisplitpane_onmouseover(fig, evt) %#ok<INUSD>
            % Fired when the mouse pointer moves over the figure window.
                if draganddrop
                    pnt = get(fig, 'CurrentPoint');
                    activesplitter.Move(pnt);
                else
                    activesplitter = uisplitter.GetActive(fig);
                    if ~isempty(activesplitter)
                        if isempty(cursor)
                            cursor = get(fig, 'Pointer');
                        end
                        switch activesplitter.Orientation
                            case 'vertical'
                                set(fig, 'Pointer', 'top');
                            otherwise
                                set(fig, 'Pointer', 'left');
                        end
                    else
                        if ~isempty(cursor)
                            set(fig, 'Pointer', cursor);
                            cursor = [];
                        end
                    end
                end
            end

            function uisplitpane_onmousedown(fig, evt) %#ok<INUSD>
            % Fired when the mouse button is pressed in the figure window.
                if ~draganddrop
                    activesplitter = uisplitter.GetActive(fig);
                    if ~isempty(activesplitter)
                        draganddrop = true;
                    end
                end
            end

            function uisplitpane_onmouseup(fig, evt) %#ok<INUSD>
            % Fired when the mouse button is released in the figure window.
                draganddrop = false;
            end
        end
    end
    methods (Access = private)
        function OnResize(self)
        % Adjust split pane and splitter dimensions.
            switch self.Orientation
                case 'vertical'
                    pos = getpixelposition(self.DividerHandle);
                    bottom = pos(2);
                    setpixelposition(self.DividerHandle, [0,bottom,pos(3),self.DividerWidth]);
                    pos = getpixelposition(self.LeftOrBottomPaneHandle);
                    setpixelposition(self.LeftOrBottomPaneHandle, [0,0,pos(3),max(1,bottom)]);
                    pos = getpixelposition(self.RightOrTopPaneHandle);
                    parent = get(self.RightOrTopPaneHandle, 'Parent');
                    parentpos = getpixelposition(parent);
                    setpixelposition(self.RightOrTopPaneHandle, [0,bottom+self.DividerWidth,pos(3),max(1,parentpos(4)-bottom-self.DividerWidth)]);
                otherwise
                    pos = getpixelposition(self.DividerHandle);
                    left = pos(1);
                    setpixelposition(self.DividerHandle, [left,0,self.DividerWidth,pos(4)]);
                    pos = getpixelposition(self.LeftOrBottomPaneHandle);
                    setpixelposition(self.LeftOrBottomPaneHandle, [0,0,max(1,left),pos(4)]);
                    pos = getpixelposition(self.RightOrTopPaneHandle);
                    parent = get(self.RightOrTopPaneHandle, 'Parent');
                    parentpos = getpixelposition(parent);
                    setpixelposition(self.RightOrTopPaneHandle, [left+self.DividerWidth,0,max(1,parentpos(3)-left-self.DividerWidth),pos(4)]);
            end
        end
    end
end