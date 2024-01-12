function collectDataHx1(dim,ssize,boot,var)

% -------------------------------------------------------------------------
% Preliminaries 
localSetup;                                                 % Run script with local setup
FILENAME = ['_D' num2str(dim) '_C' num2str(ssize)];  % Parameters of the experiment
X = getfromfile([ROOTDIR 'X'  FILENAME '.mat'],'X');
X = X(var,:);
if boot~=0
    I = getfromfile([ROOTDIR 'I' FILENAME '_B' num2str(boot) '.mat'],'I');
    X = X(:,I);
end
% Calculating the Hx data
Hx1hat = kdpee(X');
save([ROOTDIR 'Hx1hat' FILENAME '_B' num2str(boot) '_v' num2str(var) '.mat'],'Hx1hat'); 
% =========================================================================
