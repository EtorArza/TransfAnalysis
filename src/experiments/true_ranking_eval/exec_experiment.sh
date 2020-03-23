#!/bin/bash

PROBLEM_TYPE="qap"
PROBLEM_PATH="src/experiments/true_ranking_eval/tai75e01.qap"
POPSIZE=640
MAX_TIME_PSO=0.25


sbatch --export=POPSIZE=$POPSIZE,PROBLEM_TYPE=$PROBLEM_TYPE,PROBLEM_PATH=$PROBLEM_PATH,MAX_TIME_PSO=$MAX_TIME_PSO src/experiments/true_ranking_eval/exec_experiment.sl
