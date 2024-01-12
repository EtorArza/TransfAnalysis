function collectFeaturesNPK_spmd(func,dim,ssize)

% -------------------------------------------------------------------------
% Preliminaries 
localSetup;                                                 % Run script with local setup
FILENAME = ['_D' num2str(dim) '_C' num2str(ssize) '.mat'];  % Parameters of the experiment
% -------------------------------------------------------------------------
% Variables for the experiment
NPK_FEATURES = 1;
% -------------------------------------------------------------------------
% Starting the instance run.

ROOTDIR_LOCAL = ROOTDIR;
N = length(func);

spmd

    %for i=func % This allows for multiple functions with the same dimension and sample size
    for ind = 1:numlabs:N
        
        i = ind + labindex - 1;
        disp(i);
        
        if testVariableInFile([ROOTDIR_LOCAL 'NPK_F' num2str(i) FILENAME],'NPK')
            warning(['NPK_F' num2str(i) FILENAME ' already processed.'])
        else
            
            if ~exist([ROOTDIR_LOCAL 'Y_F' num2str(i) FILENAME], 'file')
                warning(['Please collect F' num2str(i) ' data. Skipped.']);
            else
                
                Y           = getfield(load([ROOTDIR_LOCAL 'Y_F' num2str(i) FILENAME]),'Y');
                MAXINSTANCE = size(Y,1);
                NPK         = zeros(MAXINSTANCE,NPK_FEATURES);
                % ---------------------------------------------------------------------
                % Instance run
                for j=1:MAXINSTANCE
                    % Calculating the HTS features
                    NPK(j,:) = numpeaksdist(Y(j,:));
                end
                % ---------------------------------------------------------------------
                % Saving the data
                if exist([ROOTDIR_LOCAL 'NPK_F' num2str(i) FILENAME], 'file')
                    savetofile([ROOTDIR_LOCAL 'NPK_F' num2str(i) FILENAME], NPK, true);
                else
                    savetofile([ROOTDIR_LOCAL 'NPK_F' num2str(i) FILENAME], NPK, false);
                end
                
            end
            
        end
    end
end
% =========================================================================
