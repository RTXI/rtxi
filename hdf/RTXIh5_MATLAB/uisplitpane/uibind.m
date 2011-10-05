function uibind(obj, event, callback)
% Registers a callback on a handle graphics object.
% This method allows registering multiple callbacks on a single handle
% graphics event hook.
%
% Input arguments:
% obj:
%    a handle graphics object
% event:
%    the name of the event for which to register the callback
% callback:
%    the callback to register, either as a function handle or as a cell
%    array

% Copyright 2010 Levente Hunyadi, http://hunyadi.info.hu

validateattributes(obj, {'numeric'}, {'scalar'});
assert(ishandle(obj), 'uibind:ArgumentTypeMismatch', ...
    'Argument expected to be a valid handle graphics object.');
validateattributes(event, {'char'}, {'row'});

hook = get(obj, event);  % get already registered callbacks if any
if isempty(hook)  % no callbacks registered yet
    set(obj, event, { @uievent ; callback });
elseif isa(hook, 'function_handle')  % a single function handle is registered
    set(obj, event, { @uievent ; hook ; callback });  % append function handle passed as argument to list of callbacks
elseif iscell(hook)
    if iscellstr(hook)
        error('uibind:CellStringCallback', ...
            'Using the deprecated syntax of specifying a callback function with a cell array of strings is not supported.');
    elseif isa(hook{1}, 'function_handle') && strcmp('uievent', func2str(hook{1}))
        set(obj, event, [ {@uievent} ; hook(2:end) ; {callback} ]);
    else
        set(obj, event, { @uievent ; hook ; callback });
    end
end