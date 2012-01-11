function uibindonce(obj, event, callback)
% Registers a callback on a handle object if not already registered.
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
%
% See also: uibind

% Copyright 2010 Levente Hunyadi, http://hunyadi.info.hu

validateattributes(obj, {'numeric'}, {'scalar'});
assert(ishandle(obj), 'uibind:ArgumentTypeMismatch', ...
    'Argument expected to be a valid handle graphics object.');
validateattributes(event, {'char'}, {'row'});
validateattributes(callback, {'function_handle'}, {'scalar'});

callbackname = func2str(callback);
hook = get(obj, event);  % get already registered callbacks if any
if isempty(hook)  % no callbacks registered yet
    set(obj, event, { @uievent ; callback });
elseif isa(hook, 'function_handle')  % a single function handle is registered
    if ~strcmp(callbackname, func2str(hook))
        set(obj, event, { @uievent ; hook ; callback });  % append function handle passed as argument to list of callbacks
    end
elseif iscell(hook)
    if iscellstr(hook)
        error('uibind:CellStringCallback', ...
            'Using the deprecated syntax of specifying a callback function with a cell array of strings is not supported.');
    elseif isa(hook{1}, 'function_handle') && strcmp('uievent', func2str(hook{1}))
        if ~uibindonce_hascallback(callbackname, hook(2:end))
            set(obj, event, [ {@uievent} ; hook(2:end) ; {callback} ]);
        end
    else
        set(obj, event, { @uievent ; hook ; callback });
    end
end

function tf = uibindonce_hascallback(callbackname, hooks)
% Tests whether a callback is among a set of callbacks.
% The callbacks are compared based on name. Only function handle callbacks
% are supported.

for k = 1 : numel(hooks)
    hook = hooks{k};
    if isa(hook, 'function_handle') && strcmp(callbackname, func2str(hook))
        tf = true;
        return;
    end
end
tf = false;