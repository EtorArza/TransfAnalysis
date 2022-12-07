The field of hyperheuristics studies the automatized generation and parametrization of optimization algorithms.
According to the literature, one of the limitations of hyperheuristics is that they are often specific to an optimization problem.

In this work, we propose a general multi-domain hyperheuristic framework applicable to a wide range of optimization problems with different search spaces.
The hyperheuristic has a neural network controller that learns to solve a given optimization problem.
The controller can then transfer the behavior learned during training to solve a previously unseen problem. 

Moreover, we propose two problem analysis methods that embed a set of optimization problems into the real space. 
These two embeddings are generated via the performance and the behavior of the hyperheuristic and are independent of the problem domain.
An experimental section in four problem sets reveals that the proposed embeddings are correlated with the properties of the optimization problems.	

## Requirements

This repo was tested on Ubuntu 20.04. 

Install c++ Linux requirements and compile:

```bash
sudo apt install libubsan0
sudo apt install gcc
sudo apt install make
sudo apt install g++

make
```

Install Python requirements:
```bash
pip install scikit-learn
pip install seaborn
pip install numpy
pip install tqdm
pip install pandas

```

## Getting started

We created a toy example in the demo folder, so that getting started is easy. In this demo, we train the hyperheuristic in the  QAP problem *demo/instances/tai75e01.qap*. and test it *demo/instances/tai60a.qap*. Note that the training is terminated at 200 seconds, and this is not enough to get a good performance, but the purpse of the demo is to show how the package works.  

We start with training the hyperheuristic

```bash
./main.out demo/train_local.ini
```

This will generate the trained controller *demo/top_controllers/qap_demo_best.controller*. Next, we test the hyperheuristic.

```bash
./main.out demo/test.ini
```

This will generate the files *responses.txt* and *score.txt*. *score.txt* contains the average objective value that the hyperheuristic obtained in the problem *demo/instances/tai60a.qap*. *responses.txt* contains the average outputs of the neural network duuring the optimization of the problem *demo/instances/tai60a.qap*. These outputs represent the summary of the behaviour of the hyperheuristic, and are the data used by the LDA as explained in the paper.




## Reproducing the results in the paper

The experiments in the paper can be replicated with the scripts inside experimentScripts/. The experimentScripts/\*.sh bash scripts were designed to be executed in our slurm based linux cluster with sbatch, and train and test the hyperheuristic. The experimentScripts/\*.py python scripts read the result files and generate the figures. 


```bash
# Generate the contour plots of the continuous functions
python experimentScripts/plot_continuous_functions.py

# Generate the Transferabioity heatmaps (analyzing problem sets via the performance of the hyperheuristic)
python experimentScripts/plot_transferability.py

# Generate the LDAs (analyzing problem sets via the behaviur of the hyperheuristic)
python experimentScripts/plot_response.py

# Comparison with parameter tuning
python experimentScripts/static_networks_grid_search_continuous.py

```




