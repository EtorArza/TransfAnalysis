function collectDataLHD(dim,ssize)

% -------------------------------------------------------------------------
% Preliminaries 
localSetup;                                                     % Run script with local setup
FILENAME     = ['_D' num2str(dim) '_C' num2str(ssize) '.mat'];  % Parameters of the experiment
VAR_LO_BOUND = -5;                                              % Variables for the experiment
VAR_RANGE    = 5 - VAR_LO_BOUND;
NBOOT = 2000;
NOBS  = dim*ssize;
% -------------------------------------------------------------------------
% Test if we still need to calculate the LHD data
flagX  = ~testVariableInFile([ROOTDIR 'X' FILENAME],'X');
if ~flagX
    warning(['X' FILENAME ' already processed.']);
end
% Test if we still need to calculate the bootstrap indexes
flagI  = ~testVariableInFile([ROOTDIR 'I' FILENAME],'I');
if ~flagI
    warning(['I' FILENAME ' already processed.']);
end
% Test if we still need to calculate the crossvalidation partition
flagCV = ~testVariableInFile([ROOTDIR 'CV' FILENAME],'CV');
if ~flagCV
    warning(['CV' FILENAME ' already processed.']);
end
% -------------------------------------------------------------------------
% Calculating the LHD data
if flagX
    rng('default');
    X = ((VAR_RANGE * lhsdesign(NOBS,dim)) + VAR_LO_BOUND)'; %#ok<*NASGU>
    save([ROOTDIR 'X' FILENAME],'X','-v7.3');
end
% Calculating the bootstrap indexes
if flagI
    rng('default'); 
    I = randi(NOBS,[NBOOT NOBS],'uint32');
    save([ROOTDIR 'I' FILENAME],'I','-v7.3');
end
% Calculating a ten-fold crosvalidation partition
if flagCV
    rng('default'); 
    CV = cvpartition(NOBS,'kfold',10);
    save([ROOTDIR 'CV' FILENAME],'CV');
    
    k = 10;
    folds = zeros(NOBS, 1);
    for ii=1:k
        folds(CV.test(ii)) = ii - 1;
    end
    save([ROOTDIR 'folds' FILENAME],'folds');
end
% =========================================================================
