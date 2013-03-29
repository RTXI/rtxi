function out = hasfield(structure,fieldname)
    fields = fieldnames(structure);
    out = false;
    for i=1:length(fields)
        if strcmp(fieldname,fields{i})
            out = true;
        end
    end
end