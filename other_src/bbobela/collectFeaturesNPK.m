function collectFeaturesNPK(func,dim,ssize)

% -------------------------------------------------------------------------
% Preliminaries 
localSetup;                                                 % Run script with local setup
FILENAME = ['_D' num2str(dim) '_C' num2str(ssize) '.mat'];  % Parameters of the experiment
% -------------------------------------------------------------------------
% Variables for the experiment
NPK_FEATURES = 1;
% -------------------------------------------------------------------------
% Starting the instance run.
t0  = clock;
for i=func % This allows for multiple functions with the same dimension and sample size
    % ---------------------------------------------------------------------
    % Test if we still need to calculate these features
    if testVariableInFile([ROOTDIR 'NPK_F' num2str(i) FILENAME],'NPK')
        warning(['NPK_F' num2str(i) FILENAME ' already processed.'])
        continue
    end
    % ---------------------------------------------------------------------
    % Test if we have the necessary data to calculate these features
    if ~filexists([ROOTDIR 'Y_F' num2str(i) FILENAME])
        warning(['Please collect F' num2str(i) ' data. Skipped.']);
        continue
    end
    Y           = getfromfile([ROOTDIR 'Y_F' num2str(i) FILENAME],'Y');
    MAXINSTANCE = size(Y,1);
    NPK         = zeros(MAXINSTANCE,NPK_FEATURES);
    % ---------------------------------------------------------------------
    % Instance run
    parfor j=1:MAXINSTANCE
        % Calculating the HTS features
        NPK(j,:) = numpeaksdist(Y(j,:));
    end
    % ---------------------------------------------------------------------
    % Saving the data
    if filexists([ROOTDIR 'NPK_F' num2str(i) FILENAME])
        save([ROOTDIR 'NPK_F' num2str(i) FILENAME],'-append','NPK');
    else
        save([ROOTDIR 'NPK_F' num2str(i) FILENAME],'NPK');
    end
    % ---------------------------------------------------------------------
    % Reporting status
    disp(['  --> NPK features have been calculated and saved. Elapsed time [h]: ' ...
          num2str(etime(clock, t0)/60/60,'%.2f')]);
end
% =========================================================================
