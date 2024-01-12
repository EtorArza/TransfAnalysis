function collectFunctionResponse(func,dim,ssize)

% -------------------------------------------------------------------------
% Preliminaries 
localSetup;                                                     % Run script with local setup
FILENAME     = ['_D' num2str(dim) '_C' num2str(ssize) '.mat'];  % Parameters of the experiment
% -------------------------------------------------------------------------
% Test if we have the necessary data to calculate these features
if ~filexists([ROOTDIR 'X'  FILENAME])
    disp('Warning: Please collect latin hypercube data.');
   return
end
% -------------------------------------------------------------------------
% Loading input values for the function
X = getfromfile([ROOTDIR 'X'  FILENAME],'X');
% Variables for the experiment
opt.algName  = 'Random Sampling for elaRobustness experiment';
opt.comments = 'Collection of data for the robustness experiment';
MAXINSTANCE  = 30;
BLOCK_SIZE   = 1e3;
NOBS = dim*ssize;
for i=func
    % -------------------------------------------------------------------------
    % Test if we still need to collect this data
%     if testVariableInFile([ROOTDIR 'Y_F' num2str(i) FILENAME],'Y')
%         warning(['Y_F' num2str(i) FILENAME ' already processed.']);
%         continue
%     end
    Y    = zeros(MAXINSTANCE,NOBS);
    % -------------------------------------------------------------------------
    % Starting the instance run.
    t0 = clock;
    for iinst=1:MAXINSTANCE
        % Seting up the BBOB generator
        fgeneric('initialize', i, iinst, COCOPATH, opt);
        if NOBS>1e4
            % To save memory, use blocks of 1000 observations at the time
            idx = 1:BLOCK_SIZE;
            for j=1:ceil(NOBS/BLOCK_SIZE)
                % Evaluate the COCO function
                Y(iinst,idx) = feval('fgeneric', X(:,idx));
                idx = idx+BLOCK_SIZE;
                % If the last block exceedes BLOCK_SIZE
                if idx(1)<NOBS && idx(BLOCK_SIZE)>NOBS
                    idx          = idx(1):NOBS;
                    Y(iinst,idx) = feval('fgeneric', X(:,idx));
                    break;
                end
            end
        else
            % Otherwise, evaluate all the elements at once.
            Y(iinst,:) = feval('fgeneric', X);
        end
        % Printing messages
        fprintf('  f%d in %d-D, instance %d: FEs=%d, elapsed time [h]: %.2f\n', ...
                 i, dim, iinst, fgeneric('evaluations'), etime(clock, t0)/60/60);
        % Closing 'fgeneric'
        fgeneric('finalize');
    end
    % Save results
    save([ROOTDIR 'Y_F' num2str(i) FILENAME],'Y');
    disp(['      date and time: ' num2str(clock, ' %.0f')]);
end
% =========================================================================
