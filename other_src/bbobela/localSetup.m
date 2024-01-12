% -------------------------------------------------------------------------
% localSetup.m
% -------------------------------------------------------------------------
% Paths for the storage of results
%ROOTDIR  = 'C:/Users/andresm/Documents/MATLAB_LOCAL/datadir/elaRobustnessVer2_Edward/';
%COCOPATH = './BBOBResultsIgnore';
%addpath('//ad.monash.edu/home/User079/andresm/Documents/MATLAB/usertool/bbob.v13.09');  % should point to fgeneric.m etc.
ROOTDIR  = '/Users/toand/git/mivp/projects/instancespace/continuous_optimization/result/';
%ROOTDIR  = 'C:\git\intancespace\continuous_optimization\result\';
COCOPATH = './BBOBResultsIgnore';
addpath('/Users/toand/git/mivp/projects/instancespace/tools/bbob.v15.03/matlab');  % should point to fgeneric.m etc.

% Auxiliary functions
getfromfile = @(filename,varname) getfield(load(filename,varname),varname);
filexists   = @(filename) exist(filename,'file');                   % The file exists
varexists   = @(filename,var) ~isempty(who('-file',filename,var));  % The variable exists
varnonzero  = @(filename,var) all(getfromfile(filename,var)~=0);    % The variable is not all zeros