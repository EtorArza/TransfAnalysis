function savetofile(fullfilename, data, append)
    if append == true
        save(fullfilename, 'data', '-append');
    else
        save(fullfilename, 'data');
    end
end