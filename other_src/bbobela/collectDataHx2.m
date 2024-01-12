function collectDataHx2(dim,ssize,boot,pair)

% -------------------------------------------------------------------------
% Preliminaries 
localSetup;                                          % Run script with local setup
FILENAME = ['_D' num2str(dim) '_C' num2str(ssize)];  % Parameters of the experiment
X = getfromfile([ROOTDIR 'X'  FILENAME '.mat'],'X');
X = X(pair,:);
if boot~=0
    I = getfromfile([ROOTDIR 'I' FILENAME '_B' num2str(boot) '.mat'],'I');
    X = X(:,I);
end
% Variables for the experiment
Hx2hat = (kdpee(X') + kdpee(flipud(X)'))./2;
save([ROOTDIR 'Hx2hat' FILENAME '_B' num2str(boot) '_a' num2str(pair(1)) '_b' num2str(pair(2)) '.mat'],'Hx2hat');
% =========================================================================
