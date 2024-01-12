function collectDataX2(dim,ssize)

% -------------------------------------------------------------------------
% Preliminaries
localSetup;                                                 % Run script with local setup
FILENAME = ['_D' num2str(dim) '_C' num2str(ssize) '.mat'];  % Parameters of the experiment
X = getfromfile([ROOTDIR 'X'  FILENAME],'X');
% Calculating the X^2 data
X2 = X.^2;
save([ROOTDIR 'X2' FILENAME],'X2','-v7.3');
% =========================================================================