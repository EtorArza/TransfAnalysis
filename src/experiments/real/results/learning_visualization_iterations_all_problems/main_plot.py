from matplotlib import pyplot as plt
import numpy as np
import subprocess
from tqdm import tqdm as tqdm
from sklearn.preprocessing import StandardScaler

SEED=2
SOLVER_POPSIZE=10
MAX_SOLVER_FE=1000
N_EVALS = 100 # N of repetitions averaged
N_REPS = 1 # N of averaged repetitions repeated
THREADS = 1
CONTROLLER_PATH = '/home/paran/Dropbox/BCAM/02_NEAT_transferability/code/NEAT_code/experimentResults/transfer_generated_continuous/controllers/all_controllers'
FULL_MODEL = "false"


instance_index_list=[0,0,0]
dim_list = [20,20,20]
NLO_train_list = [2,4,6]
NLO_test_list = [4,4,4]
CONTROLLER_PREFIX_list = [f'NLO_{NLO}' for NLO in NLO_train_list]


def write_conf_file(SOLVER_POPSIZE,PROBLEM_INDEX,PROBLEM_DIM,GEN_INDEX, CONTROLLER_PREFIX, NLO, SEED):
    subprocess.run(f"""
mkdir -p src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/
cat > src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/quad_function.dat <<EOF
1
1
{PROBLEM_DIM}
{SEED}
-1
1
{NLO}
-1
-1
-1
-1
EOF        
""",shell=True)

    
    tmp_ini_file_string = f"""
[Global] 
mode = test
PROBLEM_NAME = real_func

THREADS = {THREADS}
N_EVALS = {N_EVALS}
N_REPS = {N_REPS}

CONTROLLER_PATH = {CONTROLLER_PATH}/{CONTROLLER_PREFIX}_gen_{GEN_INDEX}.controller


PRINT_POSITIONS = false


SOLVER_POPSIZE = {SOLVER_POPSIZE}
FULL_MODEL = {FULL_MODEL}

PROBLEM_INDEX = {PROBLEM_INDEX}
PROBLEM_DIM = {PROBLEM_DIM}
MAX_SOLVER_FE = {MAX_SOLVER_FE}

"""
    with open("tmp_conf_file.ini","w") as f:
        f.writelines(tmp_ini_file_string)



def return_results(SOLVER_POPSIZE,PROBLEM_INDEX,PROBLEM_DIM,GEN_INDEX, CONTROLLER_PREFIX, NLO, SEED):
    write_conf_file(SOLVER_POPSIZE,PROBLEM_INDEX,PROBLEM_DIM,GEN_INDEX, CONTROLLER_PREFIX, NLO, SEED)
    #subprocess.run("./neat tmp_conf_file.ini > /dev/null",shell=True)
    subprocess.run("./neat tmp_conf_file.ini",shell=True)
    with open("score.txt","r") as f:
        res = f.readline().strip()
    print(SOLVER_POPSIZE,PROBLEM_INDEX,PROBLEM_DIM,GEN_INDEX, CONTROLLER_PREFIX, NLO, SEED)
    print(res)
    subprocess.run("rm score.txt",shell=True)
    return eval(res)[0][0]

j = -1
for instance_index, NLO, dim in zip(instance_index_list,NLO_train_list,dim_list):
    j += 1

    nlo_train = NLO_train_list[j]
    NLO_test = NLO_test_list[j]
    controller_prefix = CONTROLLER_PREFIX_list[j]
    dim = dim_list[j]

    files = str(subprocess.run('ls '+CONTROLLER_PATH + f"/NLO_{nlo_train}_*",shell=True, capture_output=True).stdout).split(r"\n")
    generations = [str(el.split("_gen_")[-1].split(".controller")[0]) for el in files if len(el) > 6]


    print(generations)

    results = np.zeros((len(generations), len(instance_index_list)))
    for i, gen in enumerate(tqdm(generations)):
        results[i,j] = return_results(SOLVER_POPSIZE, instance_index, dim, gen, controller_prefix, NLO_test, SEED)


    results = StandardScaler().fit_transform(results)

    for column, label in zip(results.T, instance_index_list):
        print(column, np.array([int(el) for el in generations]))
        plt.plot(np.array([int(el) for el in generations]), column, label="F"+str(label))


    plt.legend()
    plt.show()

    print(results)
