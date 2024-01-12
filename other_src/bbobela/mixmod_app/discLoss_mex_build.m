build_sys = 0; % 0: macos, 1: windows, 2: MonARCH

if build_sys == 0
    mex -v -largeArrayDims -I/usr/include -I/usr/local/include ...
    	-I/Users/toand/git/mivp/projects/instancespace/tools/libmixmod_3.2.2/INSTALL/include -cxx -c discLoss_mex.cpp
    %mex -v -largeArrayDims CXXLIBS="\$CXXLIBS -L/Users/toand/git/mivp/projects/instancespace/tools/libmixmod_3.2.2/INSTALL/lib -lmixmod -lmixmod_newmat" ...
    %	-cxx discLoss_mex.o -o discLoss_mex
    mex -v -largeArrayDims -L/Users/toand/git/mivp/projects/instancespace/tools/libmixmod_3.2.2/INSTALL/lib -lmixmod -lmixmod_newmat ...
    	-cxx discLoss_mex.o -output discLoss_mex
    %-L/usr/local/lib
    
elseif build_sys == 1
    mex -v -largeArrayDims -IC:/git/libmixmod_3.2.2/INSTALL/include -c discLoss_mex.cpp
    mex -v -largeArrayDims discLoss_mex.obj C:/git/libmixmod_3.2.2/BUILD/SRC/Release/mixmod.lib ...
        C:/git/libmixmod_3.2.2/BUILD/SRC/Release/mixmod_newmat.lib -output discLoss_mex

elseif build_sys == 2
    mex -v -largeArrayDims CXXOPTIMFLAGS="-O3 -DNDEBUG" LDOPTIMFLAGS="-O2" -I/home/toand/git/libmixmod_3.2.2/install/include -cxx -c discLoss_mex.cpp
    mex -v -largeArrayDims CXXOPTIMFLAGS="-O3 -DNDEBUG" LDOPTIMFLAGS="-O2" -L/home/toand/git/libmixmod_3.2.2/install/lib -lmixmod -lmixmod_newmat ...
        -cxx discLoss_mex.o -output discLoss_mex

end

