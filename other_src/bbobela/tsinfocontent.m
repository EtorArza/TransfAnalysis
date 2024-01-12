function Hts = tsinfocontent(Psi)
% 
epsilon = [0 logspace(-5,30,1000-1)];
s       = size(Psi,1);
psi     = bsxfun(@gt,Psi,epsilon) - bsxfun(@lt,Psi,-epsilon);
lobit   = reshape([-1 -1  0 0  1 1],[1 1 6]);
tobit   = reshape([ 0  1 -1 1 -1 0],[1 1 6]);
prob    = squeeze(sum(bsxfun(@eq,psi(1:end-1,:),lobit) & ...
                      bsxfun(@eq,psi(2:end,:)  ,tobit),1)./s);
H       = -nansum(prob.*(log(prob)./log(6)),2);
sprime  = psi(:,1);
sprime  = sprime(~(sprime==0));
M0      = sum(diff(sprime)~=0)./size(psi,1);
[Hmax,idx] = max(H);
% Hmax    = max(H);
% idx     = find(H<0.05,1,'first');
Hts     = [Hmax epsilon(idx) M0];