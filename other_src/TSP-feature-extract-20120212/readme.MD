Originally, this was a TSP feature computation code based on code by Kate Smith-Miles and Jano van
Hemert, packaged by Lin Xu. Adapted to run in Windows 10 by Etor Arza. 


This code was downloaded from the Matilda website `https://matilda.unimelb.edu.au/matilda/problems/opt/tsp#tsp`. Etor has adapted it to be able to run on Windows 10 with Matlab 2023b. I believe it was originally intended to run on Linux.

-------------------------------

## Setup 

### Installation (only once)
1) Install Matlab (I used the version 2023b).
2) Install msys2.exe. It can be downloaded from https://www.msys2.org/
3) Open C:\\mysys64\\mysy2.exe and install `ruby` with the command `pacman -S ruby`

### Set up terminal & Matlab (each time you open Matlab & mysys2.exe)
1) Open Matlab and navigate to the folder `TSP-feature-extract-20120212`
2) Open C:\\mysys64\\mysy2.exe and navigate to the folder `TSP-feature-extract-20120212` in the mysys2 terminal. In my pc, that is `cd C:\\Users\\paran\\Documents\\TransfHH\\other_src\\TSP-feature-extract-20120212\\`


-------------------------------

## Run (repeat for each TSP instance)
How to get the features for the instance `test1.tsp`. It assumes the steps in `Setup` have already been carried out.

1) Run `tspfeat2('.', 'test1.tsp')` in the Matlab command line.

2) The code execution in Matlab will pause at line 115, and it will ask you to run a command in the msys2.exe. You need to copy the command from Matlab, and paste  and run it in the mysy2.exe terminal. The command is similar to `ruby ./run_gdbscan.rb . ./tmp/test1_normal_0.82 ...`. You need to copy the command from the Matlab output, paste it on the mysy2.exe terminal and press Enter. 

3) After that command has been properly executed in the msys2.exe bash terminal, go back to the Matlab terminal and press `Enter`.

4) The result should be something like 

```
s, c_x, c_y, radius, D1fraction, D2fraction, D3fraction, D4fraction, area, sdnNNd, coeffvarnNNd, clusterratio, outlierratio, varCitiesperCluster 
102.773333, 1249.504359, 1250.109453, 1615.185155, 0.011912, 0.082865, 0.195261, 0.233833, 160000.000000, 0.106006, 0.142322, 0.001600, 0.000000, 0.000000 
```

-------------------------------


