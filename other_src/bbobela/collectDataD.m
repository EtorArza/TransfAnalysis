function collectDataD(dim,ssize)

% -------------------------------------------------------------------------
% Preliminaries 
localSetup;                                          % Run script with local setup
% Loading the required data
X = getfromfile([ROOTDIR 'X_D' dim '_C' ssize '.mat'],'X');
D = squareform(pdist(X'));
save([ROOTDIR 'D_D' dim '_C' ssize '.mat'],'D');
% =========================================================================
