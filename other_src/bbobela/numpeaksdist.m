function npeaks = numpeaksdist(Y)

NPOINTS  = 1000;
[F,Xi]   = ksdensity(Y,'npoints',NPOINTS);
minidx   = unique([1 find(F(2:NPOINTS-1) < F(1:NPOINTS-2) & ...
                          F(2:NPOINTS-1) < F(3:NPOINTS)) NPOINTS]);
intdens  = @(f,xi,a,b) mean(f(a:b))*(xi(b)-xi(a));
modemass = arrayfun(@(i) intdens(F,Xi,minidx(i),minidx(i+1)-1),1:length(minidx)-1);
npeaks   = sum(modemass > 0.01);