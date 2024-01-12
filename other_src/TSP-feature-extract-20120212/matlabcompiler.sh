#!/bin/tcsh
#
# This software is licensed under the Academic Free License version 3.0
# 
# Please refer to the full license included in LICENSE.txt
#
setenv LD_LIBRARY_PATH /cs/local/generic/lib/pkg/matlab-7.10/runtime/glnx86
matlab -nojvm -nosplash -nodesktop -nodisplay -r "addpath(genpath(pwd));mcc -m $1;exit;" 
