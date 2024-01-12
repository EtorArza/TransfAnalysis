function lvl = lvlsets(X,Y,folds)
% 
QUANTILES = [0.10 0.25 0.50];
NUMQUANTS = length(QUANTILES);
NUMFEATS  = 4;
CLASS = bsxfun(@ge,Y,quantile(Y,QUANTILES)); % Inputs by classes
lvl   = zeros(1,NUMQUANTS*NUMFEATS);

addpath('../tools/mexopencv');

for i=1:NUMQUANTS
    idx = 4*(i-1);
    save('loss_mex_test_data.mat', 'X', 'CLASS', 'folds');
    lvl(idx+1) = discLoss_mex(X, double(CLASS(:, i)), folds, 0); % 0: linear
    lvl(idx+2) = discLoss_mex(X, double(CLASS(:, i)), folds, 1); % 1: quadratic
    lvl(idx+3) = dtreeLoss_mex(X, double(CLASS(:, i)), folds);
%     tic
%     res1 = discLoss_mex(X', double(CLASS(:, i)), cv_ind, 0); % 0: linear
%     toc
%     disp(res1)
% 
%     tic
%     lvl(idx+1) = kfoldLoss(fitcdiscr(X,CLASS(:,i),'DiscrimType','linear',...
%                                      'CrossVal','on','CVPartition',CV),...
%                            'lossfun','classiferror','mode','average');
%     toc
%     disp(lvl(idx+1));
%     
%     tic
%     res2 = discLoss_mex(X', double(CLASS(:, i)), cv_ind, 1); % 1: quadratic
%     toc
%     disp(res2)
%     
%     tic
%     lvl(idx+2) = kfoldLoss(fitcdiscr(X,CLASS(:,i),'DiscrimType','quadratic',...
%                                      'CrossVal','on','CVPartition',CV),...
%                            'lossfun','classiferror','mode','average');
%     toc
%     disp(lvl(idx+2));
%     
%     tic
%     res3 = dtreeLoss_mex(X, double(CLASS(:, i)));
%     toc
%     disp(res3)
%     
%     tic
%     lvl(idx+3) = kfoldLoss(fitctree(X,CLASS(:,i),'CrossVal','on',...
%                                     'CrossVal','on','CVPartition',CV),...
%                            'lossfun','classiferror','mode','average');
%     toc
%     disp(lvl(idx+3));
    
    lvl(idx+4) = lvl(idx+1)/lvl(idx+2);
end