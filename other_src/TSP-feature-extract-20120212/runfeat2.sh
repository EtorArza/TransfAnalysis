#!/bin/tcsh
setenv LD_LIBRARY_PATH /cs/local/generic/lib/pkg/matlab-7.10/runtime/glnx86:/cs/local/generic/lib/pkg/matlab-7.10/bin/glnx86
setenv DISPLAY fake_display
# <path_to_complied_feat_matlabcode>/tspfeat2 path_to_all_scripts tsp_instance
./tspfeat2 ./ $1
