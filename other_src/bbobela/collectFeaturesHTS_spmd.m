function collectFeaturesHTS_spmd(func,dim,ssize)

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
% Starting the instance run

ROOTDIR_LOCAL = ROOTDIR;
N = length(func);

spmd    
%for i=func % This allows for multiple functions with the same dimension and sample size

    for ind = 1:numlabs:N
        
        i = ind + labindex - 1;
        disp(i);
        
        % ---------------------------------------------------------------------
        % Test if we still need to calculate these features
        if testVariableInFile([ROOTDIR_LOCAL 'HTS_F' num2str(i) FILENAME],'HTS')
            warning(['HTS_F' num2str(i) FILENAME ' already processed.'])
        else
            if ~exist([ROOTDIR_LOCAL 'Y_F' num2str(i) FILENAME], 'file')
                warning(['Please collect F' num2str(i) ' data. Skipped.']);
            else
                
                Y           = getfield(load([ROOTDIR_LOCAL 'Y_F' num2str(i) FILENAME]),'Y');
                Psi         = bsxfun(@rdivide,diff(Y(:,Sequence),1,2),DeltaX);
                MAXINSTANCE = size(Y,1);
                HTS         = zeros(MAXINSTANCE,HTS_FEATURES);

                % Instance run
                for j=1:MAXINSTANCE
                    % Calculating the HTS features
                    HTS(j,:) = tsinfocontent(Psi(j,:)');
                end

                % ---------------------------------------------------------------------
                % Save data
                if exist([ROOTDIR_LOCAL 'HTS_F' num2str(i) FILENAME], 'file')
                    savetofile([ROOTDIR_LOCAL 'HTS_F' num2str(i) FILENAME], HTS, true);
                else
                    savetofile([ROOTDIR_LOCAL 'HTS_F' num2str(i) FILENAME], HTS, false);
                end
                
            end
        end
            
    end
end
