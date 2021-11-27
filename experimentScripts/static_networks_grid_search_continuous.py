from math import atanh
from random import randint
import subprocess
import numpy as np
from tqdm import tqdm as tqdm
import time

# rm log.txt -f && cd ~/Dropbox/BCAM/02_NEAT_transferability/code/NEAT_code/ && make &&  cd .. && rsync -av --exclude=".*" NEAT_code /dev/shm/ && cd /dev/shm/NEAT_code && python experimentScripts/static_networks_grid_search_continuous.py

result_file_path = "/home/paran/Dropbox/BCAM/02_NEAT_transferability/code/NEAT_code/experimentResults/staticNetwork/"
subprocess.run(f"mkdir -p {result_file_path}",shell=True)

subprocess.run("rm -f tmp_conf_file.ini",shell=True)
subprocess.run("rm -f score.txt",shell=True)
subprocess.run("rm -f response.txt",shell=True)



SOLVER_POPSIZE=10
MAX_SOLVER_FE=1000
N_EVALS=40
N_EVALS_TEST=10000
PROBLEM_DIM=20
THREADS=6


def write_static_continuous_controller(output1, output2, output3, controller_path):
    assert -0.99 < output1 < 0.99
    assert -0.99 < output2 < 0.99
    assert -0.99 < output3 < 0.99
    str_to_write=f"""/* Organism #431 Fitness: 1.015 Error: 0.985 */
genomestart 431
trait 1 0.0745404 0 0.071647 0.306577 1 0.227115 0.795648 0.291187 
node 1 1 0
node 2 1 1
node 3 1 1
node 4 1 1
node 5 1 1
node 6 1 1
node 7 1 1
node 8 1 1
node 9 1 1
node 10 1 1
node 11 1 1
node 12 1 2
node 13 1 2
node 14 1 2
node 15 1 2
node 16 1 2
gene 1 1 12 {atanh(output1)} 0 1 1 1
gene 1 1 13 {atanh(output2)} 0 1 1 1
gene 1 1 14 {atanh(output3)} 0 1 1 1
genomeend 431
"""
    with open(controller_path, 'w') as f:
        f.write(str_to_write)


def write_conf_file_continuous(PROBLEM_INDEX,CONTROLLER_PATH, n_evals):

    tmp_ini_file_string = f"""
[Global] 
mode = test
PROBLEM_NAME = real_func

THREADS = {THREADS}
N_EVALS = {n_evals}
N_REPS = 1
CONTROLLER_PATH = {CONTROLLER_PATH}
SEED = {n_evals}

PRINT_POSITIONS = false
FULL_MODEL = false
COMPUTE_RESPONSE = true
 

SOLVER_POPSIZE = {SOLVER_POPSIZE}


PROBLEM_INDEX = {PROBLEM_INDEX}
PROBLEM_DIM = {PROBLEM_DIM}
MAX_SOLVER_FE = {MAX_SOLVER_FE}

"""

    with open("tmp_conf_file.ini","w") as f:
        f.writelines(tmp_ini_file_string)


def evaluate_continuous_static_controller(PROBLEM_INDEX, output1, output2, output3, n_evals):

    tmp_controller_name = f"tmp_{randint(1000000000000000000,9000000000000000000)}.controller"
    write_static_continuous_controller(output1, output2, output3, tmp_controller_name)
    write_conf_file_continuous(PROBLEM_INDEX,tmp_controller_name, n_evals)
    # subprocess.run("./neat tmp_conf_file.ini", shell=True) # print out
    # subprocess.run("./neat tmp_conf_file.ini > /dev/shm/NEAT_code/log.txt", shell=True) # write out into log.txt
    subprocess.run("./neat tmp_conf_file.ini > /dev/null", shell=True) # omit out
    # subprocess.run("cat score.txt >> /dev/shm/NEAT_code/log.txt",shell=True)    
    # subprocess.run("cat responses.txt >> /dev/shm/NEAT_code/log.txt",shell=True)    
    with open("score.txt","r") as f:
        res = f.readline().strip()
    score = res.split("]")[0].strip("[")
    subprocess.run("rm responses.txt",shell=True)    
    subprocess.run("rm score.txt",shell=True)
    subprocess.run(f"rm {tmp_controller_name}",shell=True)
    subprocess.run("rm tmp_conf_file.ini",shell=True)
    return score

if __name__ == "__main__":

    n_slices = 8
    n_rep_search = 3
    upper_start = 0.98
    lower_start = -0.98
    search_interval_size = upper_start - lower_start

    t = tqdm(total = n_slices*n_slices*n_slices*12*n_rep_search)
    for problem_index in range(1,13):
        upper1 = upper_start
        lower1 = lower_start
        upper2 = upper_start
        lower2 = lower_start
        upper3 = upper_start
        lower3 = lower_start
        search_interval_size = upper_start - lower_start
        best_score = -1e6
        best_outputs_and_score = []
        timeStart = time.time()
        for rep_search in range(n_rep_search):
            for output1 in np.linspace(lower1, upper1, n_slices):
                for output2 in np.linspace(lower2, upper2, n_slices):
                    for output3 in np.linspace(lower3, upper3, n_slices):
                        t.update(1)
                        score = evaluate_continuous_static_controller(problem_index,output1,output2,output3, N_EVALS)
                        if float(score) > best_score:
                            best_outputs_and_score = [problem_index, output1, output2, output3, score]
                            best_score = float(score)
            search_interval_size = search_interval_size/(n_slices-1)
            upper1 = min( 0.98, best_outputs_and_score[1] + search_interval_size * 0.5)
            lower1 = max(-0.98, best_outputs_and_score[1] - search_interval_size * 0.5)
            upper2 = min( 0.98, best_outputs_and_score[2] + search_interval_size * 0.5)
            lower2 = max(-0.98, best_outputs_and_score[2] - search_interval_size * 0.5)
            upper3 = min( 0.98, best_outputs_and_score[3] + search_interval_size * 0.5)
            lower3 = max(-0.98, best_outputs_and_score[3] - search_interval_size * 0.5)
            print(upper1, lower1, " - ", upper2, lower2, " - ", upper3, lower3)
        print(problem_index, ",", time.time() - timeStart)
        best_outputs_and_score.append(evaluate_continuous_static_controller(problem_index,output1,output2,output3, N_EVALS_TEST))
        with open(result_file_path+"continuous_grid_search.csv", "a") as f:
            print(",".join([str(el) for el in best_outputs_and_score]), file=f)


