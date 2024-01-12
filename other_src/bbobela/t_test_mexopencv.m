close all;
clear;

addpath('../tools/mexopencv');

Xtrain = randn(100,5);
labels = randi([1 3], [100 1]);
Xtest = randn(100,5);

lda = cv.LDA('NumComponents',3);
lda.compute(Xtrain, labels);
Y = lda.project(Xtest);
Xapprox = lda.reconstruct(Y);
