close all;
clear;

localSetup;                                                 % Run script with local setup
dim=10;
ssize=1000;

VAR_LO_BOUND = -5;                                              % Variables for the experiment
VAR_RANGE    = 5 - VAR_LO_BOUND;
NBOOT = 2000;
NOBS  = dim*ssize;

FILENAME = ['_D' num2str(dim) '_C' num2str(ssize) '.mat'];

%
DATA_AVAIL = 1;

if(DATA_AVAIL == 0)
    rng('default');
    tic
    X = ((VAR_RANGE * lhsdesign(NOBS,dim)) + VAR_LO_BOUND)';
    save([ROOTDIR 'X' FILENAME],'X','-v7.3');

    I = randi(NOBS,[NBOOT NOBS],'uint32');
    save([ROOTDIR 'I' FILENAME],'I','-v7.3');
    toc
else
    X     = getfromfile([ROOTDIR 'X'  FILENAME],'X');
    I     = getfromfile([ROOTDIR 'I' FILENAME],'I');
    NBOOT = size(I,1);
end

% tic
% v = kdpee(X');
% toc
% 
% tic
% collectDataAUXIN(10, 1000);
% toc
