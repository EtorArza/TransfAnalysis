close all;
clear;

localSetup;

N = 12;

tic

%collectDataSequence(10, 1000);             % 5.6s
%collectFeaturesHTS(1:N, 10, 1000);         % ~7.85s each, total: 94.39s
%collectFeaturesHTS_spmd(1:N, 10, 1000);    % ~24s each, 72s

%collectFeaturesELA(1:N, 10, 1000);         % 60s
%collectFeaturesELA_spmd(1:N, 10, 1000);    % 28s

collectFeaturesLVL(1:N, 10, 1000);         % 1745s --> 485s --> 171s (12 workers)
%collectFeaturesLVL_spmd(1:N, 10, 1000);    % 1862s 

%collectFeaturesNPK(1:N, 10, 1000);         % 12s
%collectFeaturesNPK_spmd(1:N, 10, 1000);     % 11s

toc