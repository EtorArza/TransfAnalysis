function collectBootstrapsELA(func,dim,ssize)

% -------------------------------------------------------------------------
% Preliminaries 
localSetup;                                                 % Run script with local setup
FILENAME = ['_D' num2str(dim) '_C' num2str(ssize) '.mat'];  % Parameters of the experiment
% -------------------------------------------------------------------------
% Test if we have the necessary data to calculate these features
if ~filexists([ROOTDIR 'X' FILENAME]) || ~filexists([ROOTDIR 'I' FILENAME]) || ...
        ~filexists([ROOTDIR 'XX' FILENAME]) || ~filexists([ROOTDIR 'X2' FILENAME]) || ...
        ~filexists([ROOTDIR 'Hx' FILENAME])
    warning('Please collect latin hypercube, auxiliary and indexing data. Skipped.');
    return
end
% Loading the required data
X  = getfromfile([ROOTDIR 'X'  FILENAME],'X');
XX = getfromfile([ROOTDIR 'XX' FILENAME],'XX');
X2 = getfromfile([ROOTDIR 'X2' FILENAME],'X2');
I  = getfromfile([ROOTDIR 'I' FILENAME],'I');
HxhatB  = getfromfile([ROOTDIR 'Hx' FILENAME],'HxhatB');
Hx1hatB = getfromfile([ROOTDIR 'Hx' FILENAME],'Hx1hatB');
Hx2hatB = getfromfile([ROOTDIR 'Hx' FILENAME],'Hx2hatB');
% Variables for the experiment
ELA_FEATURES = 17;
NBOOT = 2000;
DELTA = norm(10.*ones(dim,1));
% -------------------------------------------------------------------------
% Starting the function/instance run
t0     = clock;
for i=func % This allows for multiple functions with the same dimension and sample size
    % ---------------------------------------------------------------------
    % Test if we still need to calculate these features
    if filexists([ROOTDIR 'ELA_F' num2str(i) FILENAME])                                 % The file exists
        if varexists([ROOTDIR 'ELA_F' num2str(i) FILENAME],'ELABOOT')                   % The variable exists
            if all(all(varnonzero([ROOTDIR 'ELA_F' num2str(i) FILENAME],'ELABOOT')))    % The variable is not all zeros
                disp(['ELA_F' num2str(i) FILENAME ' already processed.'])
                continue
            end
        end
    end
    % ---------------------------------------------------------------------
    % Test if we have the necessary data to calculate these features
    if ~filexists([ROOTDIR 'Y_F' num2str(i) FILENAME])
        warning(['Please collect F' num2str(i) ' data. Skipped.']);
        continue
    end
    Y           = getfromfile([ROOTDIR 'Y_F' num2str(i) FILENAME],'Y');
    MAXINSTANCE = size(Y,1);
    ELABOOT     = zeros(MAXINSTANCE,ELA_FEATURES,NBOOT);
    % Instance run
    for j=1:MAXINSTANCE
        % Calculating the ELA bootstraps
        parfor k=1:NBOOT
            ELABOOT(j,:,k) = calcELA(X(:,I(k,:)),...
                                     XX(:,I(k,:)),...
                                     X2(:,I(k,:)),...
                                     Y(j,I(k,:)),...
                                     HxhatB(k),...
                                     Hx1hatB(:,k),...
                                     Hx2hatB(:,k),...
                                     DELTA); %#ok<*PFOUS,*PFBNS>
        end
        disp(['-> I' num2str(j) ' ELA bootstraps calculated, elapsed time [h]: ' num2str(etime(clock, t0)/60/60,'%.2f')]);
    end
    if filexists([ROOTDIR 'ELA_F' num2str(i) FILENAME])
        save([ROOTDIR 'ELA_F' num2str(i) FILENAME],'-append','ELABOOT');
    else
        save([ROOTDIR 'ELA_F' num2str(i) FILENAME],'ELABOOT');
    end
    disp(['      F' num2str(i) ' | date and time: ' num2str(clock, ' %.0f')]);
end
% =========================================================================
