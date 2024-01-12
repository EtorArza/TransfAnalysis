function collectFeaturesHTS(func,dim,ssize)

% -------------------------------------------------------------------------
% Preliminaries 
localSetup;                                                 % Run script with local setup
FILENAME = ['_D' num2str(dim) '_C' num2str(ssize) '.mat'];  % Parameters of the experiment
% -------------------------------------------------------------------------
% Test if we have the necessary data to calculate these features
if ~filexists([ROOTDIR 'Sequence'  FILENAME]) || ~filexists([ROOTDIR 'DeltaX'  FILENAME])
    warning('Please collect sequence data. Skipped');
    return
end
% Loading the required data
DeltaX   = getfromfile([ROOTDIR 'DeltaX' FILENAME],'DeltaX');
Sequence = getfromfile([ROOTDIR 'Sequence' FILENAME],'Sequence');
% Variables for the experiment
HTS_FEATURES = 3;
% -------------------------------------------------------------------------
% Starting the instance run.
t0  = clock;
for i=func % This allows for multiple functions with the same dimension and sample size
    % ---------------------------------------------------------------------
    % Test if we still need to calculate these features
    if testVariableInFile([ROOTDIR 'HTS_F' num2str(i) FILENAME],'HTS')
        warning(['HTS_F' num2str(i) FILENAME ' already processed.'])
        continue
    end
    % ---------------------------------------------------------------------
    % Test if we have the necessary data to calculate these features
    if ~filexists([ROOTDIR 'Y_F' num2str(i) FILENAME])
        warning(['Please collect F' num2str(i) ' data. Skipped.']);
        continue
    end
    
    Y           = getfromfile([ROOTDIR 'Y_F' num2str(i) FILENAME],'Y');
    Psi         = bsxfun(@rdivide,diff(Y(:,Sequence),1,2),DeltaX);
    MAXINSTANCE = size(Y,1);
    HTS         = zeros(MAXINSTANCE,HTS_FEATURES);
    % ---------------------------------------------------------------------
    % Instance run
    for j=1:MAXINSTANCE
        % Calculating the HTS features
        HTS(j,:) = tsinfocontent(Psi(j,:)');
    end
    % ---------------------------------------------------------------------
    % Saving the data
    if filexists([ROOTDIR 'HTS_F' num2str(i) FILENAME])
        save([ROOTDIR 'HTS_F' num2str(i) FILENAME],'-append','HTS');
    else
        save([ROOTDIR 'HTS_F' num2str(i) FILENAME],'HTS');
    end
    
    % ---------------------------------------------------------------------
    % Reporting status
    disp(['  --> HTS features have been calculated and saved. Elapsed time [h]: ' ...
          num2str(etime(clock, t0)/60/60,'%.2f')]);
end
% =========================================================================
