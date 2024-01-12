function collectFeaturesLVL(func,dim,ssize)

% -------------------------------------------------------------------------
% Preliminaries 
localSetup;                                                 % Run script with local setup
FILENAME = ['_D' num2str(dim) '_C' num2str(ssize) '.mat'];  % Parameters of the experiment
% -------------------------------------------------------------------------
% Test if we have the necessary data to calculate these features
if ~filexists([ROOTDIR 'X' FILENAME]) || ~filexists([ROOTDIR 'folds' FILENAME])
    warning('Please collect latin hypercube and folds data. Skipped');
    return
end
% Loading the required data
X  = getfromfile([ROOTDIR 'X'  FILENAME],'X');
%CV = getfromfile([ROOTDIR 'CV' FILENAME],'CV');
folds = getfromfile([ROOTDIR 'folds' FILENAME],'folds');
% Variables for the experiment
LVL_FEATURES = 12;
% -------------------------------------------------------------------------
% Starting the instance run.
t0  = clock;
for i=func % This allows for multiple functions with the same dimension and sample size
    % ---------------------------------------------------------------------
    % Test if we still need to calculate these features
%     if testVariableInFile([ROOTDIR 'LVL_F' num2str(i) FILENAME],'LVL')
%         warning(['LVL_F' num2str(i) FILENAME ' already processed.'])
%         continue
%     end
    % ---------------------------------------------------------------------
    % Test if we have the necessary data to calculate these features
    if ~filexists([ROOTDIR 'Y_F' num2str(i) FILENAME])
        warning(['Please collect F' num2str(i) ' data. Skipped.']);
        continue
    end
    Y           = getfromfile([ROOTDIR 'Y_F' num2str(i) FILENAME],'Y');
    MAXINSTANCE = size(Y,1);
    LVL         = zeros(MAXINSTANCE,LVL_FEATURES);
    % ---------------------------------------------------------------------
    % Instance run
    %parfor j=1:MAXINSTANCE
    for j=1:MAXINSTANCE
        % Calculating the HTS features
        LVL(j,:) = lvlsets(X,Y(j,:)',folds);
    end
    % ---------------------------------------------------------------------
    % Saving the data
    if filexists([ROOTDIR 'LVL_F' num2str(i) FILENAME])
        save([ROOTDIR 'LVL_F' num2str(i) FILENAME],'-append','LVL');
    else
        save([ROOTDIR 'LVL_F' num2str(i) FILENAME],'LVL');
    end
    % ---------------------------------------------------------------------
    % Reporting status
    disp(['  --> LVL features have been calculated and saved. Elapsed time [h]: ' ...
          num2str(etime(clock, t0)/60/60,'%.2f')]);
end
% =========================================================================
