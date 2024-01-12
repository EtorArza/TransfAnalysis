
clear;
load('temp.mat');
disp('Test discLoss_mex');
unix('top -l 1 | grep PhysMem:');
for ii = 1:1000
    fprintf('.');
    if( mod(ii, 50) == 0)
        fprintf('\n');
        unix('top -l 1 | grep PhysMem:');
    end
    discLoss_mex(Xinput', CLASS, folds, 0);
end
fprintf('\n');
unix('top -l 1 | grep PhysMem:');

clear;
load('tempkptree.mat');
disp('Test kdpee');
unix('top -l 1 | grep PhysMem:');
for ii = 1:1000
    fprintf('.');
    if( mod(ii, 50) == 0)
        fprintf('\n');
        unix('top -l 1 | grep PhysMem:');
    end
    kdpee(Yinput);
end
fprintf('\n');
unix('top -l 1 | grep PhysMem:');