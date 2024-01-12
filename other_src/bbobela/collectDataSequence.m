function collectDataSequence(dim,ssize)

% -------------------------------------------------------------------------
% Preliminaries 
localSetup;                                                     % Run script with local setup
FILENAME     = ['_D' num2str(dim) '_C' num2str(ssize) '.mat'];  % Parameters of the experiment
% -------------------------------------------------------------------------
% Test if we still need to calculate these features
% if filexists([ROOTDIR 'Sequence'  FILENAME])                               % The file exists
%     if varexists([ROOTDIR 'Sequence'  FILENAME],'Sequence')                % The variable exists
%         if all(all(varnonzero([ROOTDIR 'Sequence'  FILENAME],'Sequence'))) % The variable is not all zeros
%             warning(['Sequence'  FILENAME ' already processed.'])
%             return
%         end
%     end
% end
% -------------------------------------------------------------------------
% Test if we have the necessary data to calculate these features
if ~filexists([ROOTDIR 'D' FILENAME])
    if ~filexists([ROOTDIR 'X' FILENAME])
        warning('Please collect latin hypercube data. Skipped.');
        return
    end
    X = getfromfile([ROOTDIR 'X'  FILENAME],'X');
    % D = L2_distance(X,X); % Use less memory by no calculating all the distances
    nodim = true;
else
    % Loading the required data
    D       = getfromfile([ROOTDIR 'D'  FILENAME],'D');
    D(D==0) = Inf;
    nodim = false;
end

[Sequence_mex, DeltaX_mex] = collectDataSequence_mex(X);

DeltaX   = getfromfile([ROOTDIR 'DeltaX' FILENAME],'DeltaX');
Sequence = getfromfile([ROOTDIR 'Sequence' FILENAME],'Sequence');

tic

%{
% Variables for the experiment
NOBS    = dim*ssize;
% -------------------------------------------------------------------------
% Starting the experiment 
DeltaX      = zeros(1,NOBS-1);
Sequence    = zeros(1,NOBS);
Sequence(1) = 1;
if nodim
    aux = L2_distance(X(:,1),X);
    aux(aux==0) = Inf;
    aux(1)      = Inf;
else
    aux = D(1,:);
end
[DeltaX(1),Sequence(2)] = min(aux);
for i=2:NOBS-1
    if nodim
        aux = L2_distance(X(:,Sequence(i)),X);
        aux(aux==0) = Inf;
    else
        aux = D(Sequence(i),:);
    end
    aux(Sequence(1:i)) = Inf;
    [DeltaX(i),Sequence(i+1)] = min(aux);
end
save([ROOTDIR 'DeltaX' FILENAME],'DeltaX');
save([ROOTDIR 'Sequence' FILENAME],'Sequence');
% =========================================================================
%}