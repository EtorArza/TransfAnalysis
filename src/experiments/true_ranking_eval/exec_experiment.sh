#!/bin/bash

PROBLEM_TYPE="qap"
PROBLEM_PATH="src/experiments/true_ranking_eval/tai75e01.qap"
POPSIZE=1024
MAX_SOLVER_TIME=0.25
N_EVALS=20 # to be used in training
MAX_TRAIN_TIME=19800

sbatch --export=MAX_TRAIN_TIME=$MAX_TRAIN_TIME,N_EVALS=$N_EVALS,POPSIZE=$POPSIZE,PROBLEM_TYPE=$PROBLEM_TYPE,PROBLEM_PATH=$PROBLEM_PATH,MAX_SOLVER_TIME=$MAX_SOLVER_TIME src/experiments/true_ranking_eval/exec_experiment.sl
