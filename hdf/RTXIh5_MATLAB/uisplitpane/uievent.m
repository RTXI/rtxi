function uievent(obj, event, varargin)
% Invokes all registered callbacks on an object event hook.
% This function is not meant to be used directly.
%
% Input arguments:
% obj:
%    a handle graphics object
% event:
%    the event structure passed to the handle graphics object
% callbacks:
%    a cell array of function handle callbacks registered on the object

% Copyright 2010 Levente Hunyadi, http://hunyadi.info.hu

for k = 1 : numel(varargin)
    callback = varargin{k};
    if isa(callback, 'function_handle')
        callback(obj, event);
    elseif iscell(callback)
        func = callback{1};
        args = callback(2:end);
        func(obj, event, args{:});
    end
end