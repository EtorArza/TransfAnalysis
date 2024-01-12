function collectFeaturesLVL_spmd(func,dim,ssize)

% -------------------------------------------------------------------------
% Preliminaries 
localSetup;                                                 % Run script with local setup
FILENAME = ['_D' num2str(dim) '_C' num2str(ssize) '.mat'];  % Parameters of the experiment
% -------------------------------------------------------------------------
% Test if we have the necessary data to calculate these features
if ~filexists([ROOTDIR 'X' FILENAME]) || ~filexists([ROOTDIR 'CV' FILENAME])
    warning('Please collect latin hypercube and crossvalidation data. Skipped');
    return
end
% Loading the required data
X  = getfromfile([ROOTDIR 'X'  FILENAME],'X');
CV = getfromfile([ROOTDIR 'CV' FILENAME],'CV');
% Variables for the experiment
LVL_FEATURES = 12;
% -------------------------------------------------------------------------
% Starting the instance run.

ROOTDIR_LOCAL = ROOTDIR;
N = length(func);

spmd
%for i=func % This allows for multiple functions with the same dimension and sample size

    for ind = 1:numlabs:N
        
        i = ind + labindex - 1;
        disp(i);
        
        % ---------------------------------------------------------------------
        % Test if we still need to calculate these features
        if testVariableInFile([ROOTDIR_LOCAL 'LVL_F' num2str(i) FILENAME],'LVL')
            warning(['LVL_F' num2str(i) FILENAME ' already processed.'])
        else
            if ~exist([ROOTDIR_LOCAL 'Y_F' num2str(i) FILENAME], 'file')
                warning(['Please collect F' num2str(i) ' data. Skipped.']);
            else
                Y           = getfield(load([ROOTDIR_LOCAL 'Y_F' num2str(i) FILENAME]),'Y');
                MAXINSTANCE = size(Y,1);
                LVL         = zeros(MAXINSTANCE,LVL_FEATURES);
                % ---------------------------------------------------------------------
                % Instance run
                for j=1:MAXINSTANCE
                    % Calculating the HTS features
                    LVL(j,:) = lvlsets(X',Y(j,:)',CV);
                end
                % ---------------------------------------------------------------------
                % Saving the data
                if exist([ROOTDIR_LOCAL 'LVL_F' num2str(i) FILENAME], 'file')
                    savetofile([ROOTDIR_LOCAL 'LVL_F' num2str(i) FILENAME], LVL, true);
                else
                    savetofile([ROOTDIR_LOCAL 'LVL_F' num2str(i) FILENAME], LVL, false);
                end
            end
        end
    end
end
% =========================================================================
