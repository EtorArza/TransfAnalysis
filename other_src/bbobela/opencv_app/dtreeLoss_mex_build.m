build_sys = 2; % 0: macos, 1: windows, 2: monarch

if build_sys == 0
    mex -v -largeArrayDims -I/usr/include -I/usr/local/include -cxx -c dtreeLoss_mex.cpp
    mex -v -largeArrayDims CXXLIBS="\$CXXLIBS -lopencv_core -lopencv_ml" -cxx dtreeLoss_mex.o -o dtreeLoss_mex

elseif build_sys == 1
    mex -v -largeArrayDims -IC:/opencv/2.4.13/build/include -c dtreeLoss_mex.cpp
    mex -v -largeArrayDims dtreeLoss_mex.obj C:/opencv/2.4.13/build/x64/vc12/lib/opencv_core2413.lib ...
        C:/opencv/2.4.13/build/x64/vc12/lib/opencv_ml2413.lib -output dtreeLoss_mex

elseif build_sys == 2
    mex -v -largeArrayDims CXXOPTIMFLAGS="-O3 -DNDEBUG" LDOPTIMFLAGS="-O2" -I/home/toand/git/opencv-2.4.13/build/install/include -cxx -c dtreeLoss_mex.cpp
    mex -v -largeArrayDims CXXOPTIMFLAGS="-O3 -DNDEBUG" LDOPTIMFLAGS="-O2" -L/home/toand/git/opencv-2.4.13/build/install/lib -lopencv_core -lopencv_ml -cxx dtreeLoss_mex.o -output dtreeLoss_mex

end
