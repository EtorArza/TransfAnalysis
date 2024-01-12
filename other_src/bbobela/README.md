[![DOI](https://zenodo.org/badge/211958367.svg)](https://zenodo.org/badge/latestdoi/211958367)


This is perhaps more complex that the project before. The data flow usually is as follow:

- Make sure that you modify the local configuration by changing localSetup.m
- Given a dimension (dim=2) and a sample size (ssize=1000), collect the X, I, XX, X2 and Hx values using collectDataAUXIN.m 
    In this file it is missing the collection of CV which is in collectDataLHD.m, 
    and there are a number of individual files that collect the other variables, e.g. collectDataX2.m
- Generate a sequence using collectDataSequence.m 
    This file has also the potential to save a distance matrix D, but it is usually not done due to memory/hdd constraints
- Using X, calculate the response of the functions. 
    This step is very function dependent, and we have no much control over it, 
    so you should not focus on this part. An example of how this is done is presented in collectFunctionResponse.m 
    which uses an external library called BBOB/COCO [http://coco.gforge.inria.fr/] 
    (I've used v13.09, but probably any version would work). 
    Using the X in the data folder, all the responses for this functions are on the Y_F?_D2_C1000.m files.
- Calculate the meta features with the functions collectFeaturesELA.m, collectFeaturesHTS.m, 
    collectFeaturesLVL.m and collectFeaturesNPK.m - Some of this functions will call additional functions that actually do the processing, 
    e.g. collectFeaturesHTS.m calls tsinfocontent.m
- Now, the results from all these programs are available in the data folder. 


I would think that the most important tasks are:

- Examine the mex function kdpeemex, which is used in steps 2 and 5. 
    Improving this algorithm n-dimensional entropy estimator will certainly speed up the computation.
- Examine the meta feature functions (step 6) and check how they can be improved.
- Improve the sequencing algorithm used in step 3. The current one is a very simple implementation of 
    nearest neighbor sequencing using a taboo list.
