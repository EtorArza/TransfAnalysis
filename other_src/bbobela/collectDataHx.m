function collectDataHx(dim,ssize,boot)

% -------------------------------------------------------------------------
% Preliminaries 
localSetup;                                                 % Run script with local setup
FILENAME = ['_D' num2str(dim) '_C' num2str(ssize)];  % Parameters of the experiment
X = getfromfile([ROOTDIR 'X'  FILENAME '.mat'],'X');
if boot~=0
    I = getfromfile([ROOTDIR 'I' FILENAME '_B' num2str(boot) '.mat'],'I');
    X = X(:,I);
end
Hxhat = (kdpee(X')+kdpee(flipud(X)'))./2;
save([ROOTDIR 'Hxhat' FILENAME '_B' num2str(boot) '.mat'],'Hxhat');    
% =========================================================================
