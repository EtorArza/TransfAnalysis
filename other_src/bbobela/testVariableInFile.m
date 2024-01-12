function [flag,varvalue] = testVariableInFile(filename,varname)

flag = false;
varvalue = [];
if exist(filename,'file')                       % The file exists
    if ~isempty(who('-file',filename,varname))  % The variable exists
        varvalue = getfield(load(filename,varname),varname);
        if (isnumeric(varvalue) && ~all(varvalue(:)==0)) || isa(varvalue,'cvpartition')
            flag = true;
        end
    end
end
