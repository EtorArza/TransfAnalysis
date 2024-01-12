function collectDataXX(dim,ssize)

% -------------------------------------------------------------------------
% Preliminaries
localSetup;                                                 % Run script with local setup
FILENAME = ['_D' num2str(dim) '_C' num2str(ssize) '.mat'];  % Parameters of the experiment
X = getfromfile([ROOTDIR 'X'  FILENAME],'X');
% Variables for the experiment
idx = nchoosek(1:dim,2);
% Calculating the XxX data
XX = X(idx(:,1),:).*X(idx(:,2),:);
save([ROOTDIR 'XX' FILENAME],'XX','-v7.3');
% =========================================================================
