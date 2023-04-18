from math import atanh
from random import randint
import subprocess
import numpy as np
from tqdm import tqdm as tqdm
import time



# rm log.txt -f && cd ~/Dropbox/BCAM/02_NEAT_transferability/code/NEAT_code/ && make &&  cd .. && rsync -av --exclude=".*" NEAT_code /dev/shm/ && cd /dev/shm/NEAT_code && python experimentScripts/static_networks_grid_search_continuous.py

result_file_path = "experimentResults/staticNetwork/"
subprocess.run(f"mkdir -p {result_file_path}",shell=True)

subprocess.run("rm -f tmp_conf_file.ini",shell=True)
subprocess.run("rm -f score.txt",shell=True)
subprocess.run("rm -f response.txt",shell=True)



SOLVER_POPSIZE=8
MAX_SOLVER_FE=400
N_EVALS_TRAIN=400
N_EVALS_TEST=10000
PROBLEM_DIM=20
THREADS=6
n_slices=20
TRAIN_SEED = 73262
TEST_SEED = 3
max_restarts_after_crash_rokkonen = 300


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


def write_conf_file_continuous(PROBLEM_INDEX,CONTROLLER_PATH, n_evals, seed,nlo):
    if PROBLEM_INDEX==0:
        with open("src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/quad_function.dat", "w") as f:
            f.writelines(
f"""1
1
{PROBLEM_DIM}
{seed}
-1
1
{nlo}
-1
-1
-1
-1
EOF
""")

    tmp_ini_file_string = f"""
[Global] 
mode = test
PROBLEM_NAME = real_func

THREADS = {THREADS}
N_EVALS = {n_evals}
N_REPS = 1
CONTROLLER_PATH = {CONTROLLER_PATH}
SEED = {seed}

PRINT_POSITIONS = false
FULL_MODEL = false
COMPUTE_RESPONSE = false
 

SOLVER_POPSIZE = {SOLVER_POPSIZE}


PROBLEM_INDEX = {PROBLEM_INDEX}
PROBLEM_DIM = {PROBLEM_DIM}
MAX_SOLVER_FE = {MAX_SOLVER_FE}

"""

    with open("tmp_conf_file.ini","w") as f:
        f.writelines(tmp_ini_file_string)


def evaluate_continuous_static_controller(PROBLEM_INDEX, output1, output2, output3, n_evals, seed):

    tmp_controller_name = f"tmp_{randint(1000000000000000000,9000000000000000000)}.controller"
    write_static_continuous_controller(output1, output2, output3, tmp_controller_name)
    f = evaluate_with_controller(PROBLEM_INDEX, tmp_controller_name, n_evals, seed)
    subprocess.run(f"rm {tmp_controller_name}",shell=True)
    return f

def evaluate_with_controller(problem_index, controller_path, n_evals, seed):

    nlo = None

    if problem_index >= 12:
        nlo = [1, 2, 4, 8, 16, 32, 64][problem_index-12]
        problem_index = 0
    else:
        problem_index += 1

    # print(f"Working on problem {problem_index} and nlo =",nlo)
    # print(f"Controller: ", controller)
    
    write_conf_file_continuous(problem_index, controller_path, n_evals, seed, nlo)
    # subprocess.run("./main.out tmp_conf_file.ini", shell=True) # print out
    # subprocess.run("./main.out tmp_conf_file.ini > /dev/shm/NEAT_code/log.txt", shell=True) # write out into log.txt
    subprocess.run("./main.out tmp_conf_file.ini > /dev/null", shell=True) # omit out
    # subprocess.run("cat score.txt >> /dev/shm/NEAT_code/log.txt",shell=True)    
    # subprocess.run("cat responses.txt >> /dev/shm/NEAT_code/log.txt",shell=True)    
    with open("score.txt","r") as f:
        res = f.readline().strip()
    score = res.split("]")[0].strip("[")
    subprocess.run("rm score.txt",shell=True)
    subprocess.run("rm tmp_conf_file.ini",shell=True)
    return float(score)

def get_hh_controller_path(index, seed):
    hh_controller_paths = [f"experimentResults/transfer_16_continuous_problems/controllers/top_controllers/TrainOnlyInF_{j+1}_seed{seed}_best.controller" for j in range(12)] + [f"experimentResults/transfer_generated_continuous/controllers/top_controllers/NLO_{j}_seed{seed}_best.controller" for j in [1, 2, 4, 8, 16, 32, 64]]
    return hh_controller_paths[index]


if __name__ == "__main__":

    problem_index_labels = [f"A_{i}" for i in range(1,13)] + [f"nlo_{i}" for i in [1,2,4,8,16,32]]

    n_rep_search = 1
    upper_start = 0.98
    lower_start = -0.98
    all_rows = []
    t = tqdm(total = n_slices*n_slices*n_slices*19*n_rep_search)
    for problem_index in range(18):
        row_to_be_written = [problem_index_labels[problem_index]]

        # Default params
        hack_to_compatibilize = lambda x: x / 2
        output1,output2,output3 = hack_to_compatibilize(0.729844), hack_to_compatibilize(1.49618), hack_to_compatibilize(1.49618)
        done_test=False
        test_seed = TEST_SEED
        while not done_test and test_seed < max_restarts_after_crash_rokkonen:
            print("Evaluating ...")
            try:
                subprocess.run("rm -f tmp_conf_file.ini",shell=True)
                subprocess.run("rm -f score.txt",shell=True)
                subprocess.run("rm -f response.txt",shell=True)
                f_reeval = evaluate_continuous_static_controller(problem_index,output1,output2,output3, N_EVALS_TEST, test_seed)
                row_to_be_written += [f_reeval]
                done_test=True
            except FileNotFoundError:
                print("Failed with seed ", test_seed,"!")
                print("Trying with next seed...")
                test_seed += 1



        # Grid search
        upper1 = upper_start
        lower1 = lower_start
        upper2 = upper_start
        lower2 = lower_start
        upper3 = upper_start
        lower3 = lower_start
        best_score = -1e6
        timeStart = time.time()
        for output1 in np.linspace(lower1, upper1, n_slices):
            for output2 in np.linspace(lower2, upper2, n_slices):
                for output3 in np.linspace(lower3, upper3, n_slices):
                    t.update(1)
                    score = evaluate_continuous_static_controller(problem_index,output1,output2,output3, N_EVALS_TRAIN, TRAIN_SEED)
                    if float(score) > best_score:
                        best_outputs = [output1, output2, output3]
                        best_score = float(score)

        test_seed = TEST_SEED
        done_test=False
        output1,output2,output3 = best_outputs[0],best_outputs[1],best_outputs[2]
        while not done_test and test_seed < max_restarts_after_crash_rokkonen:
            print("Evaluating ...")
            try:
                subprocess.run("rm -f tmp_conf_file.ini",shell=True)
                subprocess.run("rm -f score.txt",shell=True)
                subprocess.run("rm -f response.txt",shell=True)
                f_reeval = evaluate_continuous_static_controller(problem_index,output1,output2,output3, N_EVALS_TEST, test_seed)
                row_to_be_written+= [f_reeval]
                done_test=True
            except FileNotFoundError:
                print("Failed with seed ", test_seed,"!")
                print("Trying with next seed...")
                test_seed += 1


        # Hyper-heuristic
        best_controller_path = None
        best_train_f = -1e20
        all_train_f = []
        for controller_seed in range(2, 12):
            controller = get_hh_controller_path(problem_index, controller_seed)
            f_train = evaluate_with_controller(problem_index, controller, N_EVALS_TRAIN, TRAIN_SEED)
            all_train_f.append(f_train)
            if f_train > best_train_f:
                best_train_f = f_train
                best_controller_path = controller
        test_f = evaluate_with_controller(problem_index, best_controller_path, N_EVALS_TEST, TEST_SEED)
        row_to_be_written += [test_f]
        print(problem_index, test_f)

        all_rows += [row_to_be_written]
        # Problem name, Default params, Grid search, HH
        with open(result_file_path+"continuous_grid_search.csv", "a") as f:
            print(",".join([str(el) for el in row_to_be_written]), file=f)

    def argmax_ignorefirst(lst):
      return lst.index(max(lst[1:]))
    def add_bold(txt):
        return r"\textbf{" + txt + r"}"
    def row_to_tex(row):
        from copy import deepcopy
        rowlist = deepcopy(row)
        idx_max = argmax_ignorefirst(rowlist)
        rowlist[1:] = ['{:.5f}'.format(el) for el in rowlist[1:]]
        rowlist[idx_max] = add_bold(str(rowlist[idx_max]))
        rowlist = [str(el) for el in rowlist]
        return " & ".join(rowlist) + r"\\"

    for row in all_rows:
        with open(result_file_path+"continuous_grid_search_tex.csv", "a") as f:
            print(row_to_tex(row), file=f)
