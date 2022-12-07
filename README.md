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

Install c++ Linux requirements:

```bash
sudo apt install libubsan0
sudo apt install gcc
sudo apt install make
sudo apt install g++
```

Install Python requirements:
```python
pip install scikit-learn
pip install seaborn
pip install numpy
pip install tqdm
pip install pandas

```


## Reproducing the results in the paper

The experiments in the paper can be replicated with the scripts inside experimentScripts/. The experimentScripts/\*.sh bash scripts were designed to be executed in our slurm based linux cluster, and train and test the hyperheuristic. The experimentScripts/\*.py python files read this data and generate the figures.







