build_sys = 0; % 0: mac, 1:win

if(build_sys == 0)
    mex -v -largeArrayDims -DUSE_OpenMP CXXFLAGS="\$CXXFLAGS -fopenmp" CXXLIBS="\$CXXLIBS -fopenmp" -cxx collectDataSequence_mex.cpp ...
        -o collectDataSequence_mex

elseif(build_sys == 1)
    mex -v -largeArrayDims -DUSE_OpenMP COMPFLAGS="/openmp $COMPFLAGS" collectDataSequence_mex.cpp -output collectDataSequence_mex
end