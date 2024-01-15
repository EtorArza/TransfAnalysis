%
%   This software is licensed under the Academic Free License version 3.0
%
%   Please refer to the full license included in LICENSE.txt
%
% This script is to find the following metrics when clustering algorithm DBSCAN has min size = 5:
% (1) Mean S.D. of distances bet a city & all other cities (=m)
% (2) Centroid of TSP ID (xcoord) (=c_x)
% (3) Centroid of TSP ID (ycoord) (=c_y)
% (4) Radius of TSP ID (=D_i)
% (5) Fraction of distinct distances in 100 x 100 matrix D,  no. of digits =1 (=u_1)
% (6) Fraction of distinct distances in 100 x 100 matrix D, no. of digits=2 (=u_2)
% (7) Fraction of distinct distances in 100 x 100 matrix D, no. of digits =3 (=u_3)
% (8) Fraction of distinct distances in 100 x 100 matrix D, no. of digits =4 (=u_4)
% (9) Rectangular area (units) (=area)
% (10) Normalised variance of nNNds (=v)
% (11) Coefficient of variation of normalised nNNds (V=100*(s.d./mean)%) (=V)
% (12) No. of clusters (min size=5) (=max_5)
% (13) Distribution of no. of cities in each cluster (min size=5) (=counts)
% (14) Variance of no. of cities in each cluster (min size=5) (=varr)
% (15) No. of outliers (min size=5) (=outlier)
% (16) Ratio of nodes near edges of plane (min size=5) (=edge)


function tspfeat2(mypath, tspname)

% Check if the file exists
if exist(tspname, 'file') ~= 2
    error('Error: The specified .tsp file does not exist.');
end

[pathstr, name, ext] = fileparts(tspname); 



% [mypath, ff1, ff2] = fileparts(which('tspfeat2'));
seed=rand();

sfile=sprintf('./tmp/%s_normal_%f', name, seed);
gdbfile1=sprintf('./tmp/%s_gdb1_%f', name, seed);
gdbfile2=sprintf('./tmp/%s_gdb2_%f', name, seed);
gdbfile3=sprintf('./tmp/%s_gdb3_%f', name, seed);
% jfile=sprintf('./tmp/%s_jfile_%f', name, seed);
mycmd=sprintf('!ruby %s/modifytspinstance.rb %s %s', mypath, tspname, sfile);
eval(mycmd);

fid=fopen(sfile);
tline = fgetl(fid);
data=[];

while ischar(tline)
    c = textscan(tline,'%f%f','delimiter', ',');
    data=[data; [c{1}, c{2}]];
    tline=fgetl(fid);
end    
fclose(fid);

%            data = dlmread(s, '', [6 /0 105 2]); % reads .tsp file from corner (6,0) to (105,2) where first row is 0
data

x=data(:,1);
y=data(:,2);
area =(max(x)-min(x))*(max(y)-min(y));
d=[x,y];


pos=d';
D=dist(pos); % calculates the distance matrix between each of the cities

D1=round(D*10)/10; % rounds inter-city distances to the nearest 1 decimal place, etc.
D2=round(D*100)/100;
D3=round(D*1000)/1000;
D4=round(D*10000)/10000;

D1fraction=size(unique(D1),1)/(size(D1,1)*size(D1,1)); %fraction of distinct intercity distances
D2fraction=size(unique(D2),1)/(size(D2,1)*size(D2,1));
D3fraction=size(unique(D3),1)/(size(D3,1)*size(D3,1));
D4fraction=size(unique(D4),1)/(size(D4,1)*size(D4,1));

Dsort=sort(D,'descend');
Top5=Dsort([1:5],:);
maxdist=max(max(D));
Top5=Top5/maxdist;
sdnNNd=std2(Top5);
meannNNd=mean(mean(Top5));
coeffvarnNNd=sdnNNd/meannNNd;


s=std2(D); % computes sd of elements of intercity distances matrix
c_x = 0.01*sum(x);
c_y = 0.01*sum(y);
centroid = [0.01*sum(x),0.01*sum(y)];

%D = sum((x-y).^2).^0.5 %calculates a matrix of distances between each city and centroid
for j=1:100
    CentroidDistance(j)=(((d(j,1)-centroid(1)).^2)+((d(j,2)-centroid(2)).^2)).^0.5;
end;
radius=mean(CentroidDistance);

% 
% s2=['H:\Kate files\Research\meta-learning\TSP\Jano\CLKeasy\' name '.gdbscan'];
% outliers = dlmread(s2, '', [1 1 1 1]);
% clusters=dlmread(s2, '', [2 1 2 1]);
% varcol=14+clusters;
% if clusters==1 varCitiesperCluster=0; else varCitiesperCluster=dlmread(s2, '', [varcol 1 varcol 1]); end;
% 
% clusterratio=clusters/100;
% outlierratio=outliers/100;
% 


mycmd=sprintf('ruby %s/run_gdbscan.rb %s %s %s %s %s', mypath, mypath,sfile, gdbfile1, gdbfile2, gdbfile3);
disp('run')
disp(' ')
disp(' ')
disp(mycmd)
disp(' ')
disp(' ')
disp('on MINGW64 bash terminal. Make sure you are on the folder TSP-feature-extract-20120212')
disp(' ')
input('Press Enter to continue once you have run the command...');
% system(mycmd);


linresult1=csvread(gdbfile1);
linresult2=csvread(gdbfile2);
linresult3=csvread(gdbfile3);

% mycmd=sprintf('!%s/jstats -a -i 99 %s |cut -f 2 -d '' '' |head -n 15 > %s', mypath, gdbfile3, jfile);
% eval(mycmd)


outliers = linresult2(1);
clusters= linresult2(2);
numnode=linresult2(3);
varCitiesperCluster=0;
if clusters==0
    varCitiesperCluster=-512; 
else
    if clusters>1
 %     foo=csvread(jfile);
 %      varCitiesperCluster=foo(11);
      varCitiesperCluster=var(linresult3);
    end
end;
 
 clusterratio=clusters/numnode;
 outlierratio=outliers/numnode;

% M =[s c_x c_y radius D1fraction D2fraction D3fraction D4fraction area sdnNNd coeffvarnNNd];


M =[s c_x c_y radius D1fraction D2fraction D3fraction D4fraction area sdnNNd coeffvarnNNd clusterratio outlierratio varCitiesperCluster];
nameM={'s' 'c_x' 'c_y' 'radius' 'D1fraction' 'D2fraction' 'D3fraction' 'D4fraction' 'area' 'sdnNNd' 'coeffvarnNNd' 'clusterratio' 'outlierratio' 'varCitiesperCluster'};

for i =1:length(M)-1
fprintf('%s,', nameM{i});
end
fprintf('%s\n', nameM{end})';

fprintf('%s%s,', name, ext);
for i =1:length(M)-1
fprintf('%f,', M(i));
end
fprintf('%f\n', M(end))';

delete(sfile);
delete(gdbfile1);
delete(gdbfile2);
delete(gdbfile3);
% delete(jfile);
end
