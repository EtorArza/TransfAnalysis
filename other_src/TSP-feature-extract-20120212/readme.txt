TSP feature computation code based on code by Kate Smith-Miles and Jano van
Hemert, packaged by Lin Xu.  You can either use this code from within Matlab
(the easy option) or compile the Matlab code and run it from the command line
(the more complicated option that is, however, more convenient for using the
code inside other scripts).

core files:
 gdbscan.pl  
 modifytspinstance.rb 
 tspfeat2.m 
 modifytspinstance.rb  

matlab compiler tool:
 matlabcompiler.sh  # you may need to change this file for your machine

run feat:
 runfeat2.sh  # you may need to change this file for your machine

======================================

First option: from within Matlab.
Example call: 
tspfeat2('.', 'test1.tsp')

======================================

Second option: from the command line.

First compile matlab code using:
   matlabcompiler.sh tspfeat2.m

Then run:
  runfeat2.sh test1.tsp

===================================

For any questions, please contact Lin Xu at: xulin730@cs.ubc.ca

