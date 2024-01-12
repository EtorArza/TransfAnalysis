#!/bin/bash
matlab -nojvm -nosplash -nodesktop -r "addpath(genpath(pwd));mcc -m ./tspfeat2.m;exit;" 
