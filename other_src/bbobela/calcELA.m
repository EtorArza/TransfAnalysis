function ELA = calcELA(X,XX,X2,Y,Hxhat,Hx1hat,Hx2hat,DELTA)
% 
DIM     = size(X,1);
NOBS    = size(X,2);
PCENT   = 0.01;
ELA     = zeros(1,17);
COLONES = ones(NOBS,1);
Yavg    = mean(Y);
% Fitness distance correlation function. There is only one measure.
[~,idx] = sort(Y);
D       = L2_distance(X,X(:,idx(1)));
ELA(1)  = corr(Y',D);
% Dispersion at 1%. There is only one measure.
auxidx  = idx(1:ceil(PCENT.*NOBS));
D       = L2_distance(X(:,auxidx),X(:,auxidx));
ELA(2)  = mean(D(~eye(size(D))))./DELTA;
% Linear model
Xmodel  = [COLONES X'];
[Q,R]   = qr(Xmodel,0);
beta    = R\(Q'*Y');
Yhat    = Xmodel*beta;
ELA(3)  = 1 - (sum((Y' - Yhat).^2,1)./sum((Y'-Yavg).^2,1))*((NOBS-1)./(NOBS-size(beta,1)));
ELA(7)  = min(abs(beta(2:end)));
ELA(8)  = max(abs(beta(2:end)));
% Linear model plus interactions
Xmodel  = [COLONES X' XX'];
[Q,R]   = qr(Xmodel,0);
beta    = R\(Q'*Y');
Yhat    = Xmodel*beta;
ELA(4)  = 1 - (sum((Y' - Yhat).^2,1)./sum((Y'-Yavg).^2,1))*((NOBS-1)./(NOBS-size(beta,1)));
% Quadratic model
Xmodel  = [COLONES X' XX' X2'];
[Q,R]   = qr(Xmodel,0);
beta    = R\(Q'*Y');
Yhat    = Xmodel*beta;
ELA(5)  = 1 - (sum((Y' - Yhat).^2,1)./sum((Y'-Yavg).^2,1))*((NOBS-1)./(NOBS-size(beta,1)));
% Pure quadratic model
Xmodel  = [COLONES X' X2'];
[Q,R]   = qr(Xmodel,0);
beta    = R\(Q'*Y');
Yhat    = Xmodel*beta;
ELA(6)  = 1 - (sum((Y' - Yhat).^2,1)./sum((Y'-Yavg).^2,1))*((NOBS-1)./(NOBS-size(beta,1)));
ELA(9)  = min(abs(beta(end-(DIM-1):end)))/max(abs(beta(end-(DIM-1):end)));
% Entropic significance
xi1     = zeros(1,DIM);         % Significance of each individual variable
Hyhat   = kdpee(Y');
for k=1:DIM
    xi1(k) = significance([Y; X(k,:)],Hyhat,Hx1hat(k));
end
m     = nchoosek(1:DIM,2);     % All possible combinations of two variables
xi2   = zeros(1,size(m,1));
for i=1:size(m,1)
    xi2(i) = significance([Y; X(m(i,:),:)],Hyhat,Hx2hat(i));
end
xin        = significance([Y; X],Hyhat,Hxhat);
ELA(10:15) = [Hyhat xin mean(xi1) std(xi1) mean(xi2) std(xi2)];
% Cost distributions
ELA(16) = skewness(Y);
ELA(17) = kurtosis(Y);
% =========================================================================
function xi = significance(input,Hyhat,Hxhat)
% 
xi     = (Hxhat + Hyhat - kdpee(input'))/Hyhat;
% =========================================================================