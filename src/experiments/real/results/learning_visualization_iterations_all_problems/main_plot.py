from matplotlib import pyplot as plt
import numpy as np
import subprocess
from tqdm import tqdm as tqdm
from sklearn.preprocessing import StandardScaler

SEED=2
SOLVER_POPSIZE=20
MAX_SOLVER_FE=200000
DIM=10
N_EVALS = 100 # N of repetitions averaged
N_REPS = 1 # N of averaged repetitions repeated
THREADS = 16
CONTROLLER_PATH = 'src/experiments/real/results/learning_visualization_iterations_all_problems/all_controllers/'
CONTROLLER_PREFIX = 'trained_with_all_problems_true'
N_PROBLEMS = 10
FULL_MODEL = "true"


instance_list=[1,2,3,4,5,6,7,8,9,10]
lower_list=[-10,-10,-5,-5.12,-15,-600,-5,-15,-100,-2.048]
upper_list=[5,10,10,5,30,600,10,30,100,2.048]
dim_list = [10]*10

def write_conf_file(SOLVER_POPSIZE,PROBLEM_INDEX,PROBLEM_DIM,X_LOWER_LIM,X_UPPER_LIM,GEN_INDEX):

    values = {
        "SOLVER_POPSIZE":SOLVER_POPSIZE,
        "PROBLEM_INDEX":PROBLEM_INDEX,
        "PROBLEM_DIM":PROBLEM_DIM,
        "MAX_SOLVER_FE":MAX_SOLVER_FE,
        "X_LOWER_LIM":X_LOWER_LIM,
        "X_UPPER_LIM":X_UPPER_LIM,
        "CONTROLLER_PATH":CONTROLLER_PATH,
        "N_EVALS":N_EVALS,
        "N_REPS":N_REPS,
        "THREADS":THREADS,
        "GEN_INDEX":str(GEN_INDEX).zfill(9),
        "FULL_MODEL": FULL_MODEL,
        "CONTROLLER_PREFIX": CONTROLLER_PREFIX,
    }

    tmp_ini_file_string = """
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

X_LOWER_LIM = {X_LOWER_LIM}
X_UPPER_LIM = {X_UPPER_LIM}
""".format(**values)

    with open("tmp_conf_file.ini","w") as f:
        f.writelines(tmp_ini_file_string)



def return_results(SOLVER_POPSIZE,PROBLEM_INDEX,PROBLEM_DIM,X_LOWER_LIM,X_UPPER_LIM,GEN_INDEX):
    write_conf_file(SOLVER_POPSIZE,PROBLEM_INDEX,PROBLEM_DIM,X_LOWER_LIM,X_UPPER_LIM,GEN_INDEX)
    subprocess.run("./neat tmp_conf_file.ini > /dev/null",shell=True)
    # subprocess.run("./neat tmp_conf_file.ini",shell=True)
    with open("result.txt","r") as f:
        res = f.readline().strip()
    subprocess.run("rm result.txt",shell=True)
    return eval(res)[0][0]


files = str(subprocess.run('ls '+CONTROLLER_PATH,shell=True, capture_output=True).stdout).split(r"\n")
generations = [int(el.split("_gen_")[-1].split(".controller")[0]) for el in files if len(el) > 6]



results = np.zeros((len(generations), len(instance_list)))
for i, gen in enumerate(tqdm(generations)):
    for j, problem_index in enumerate(instance_list):
        X_LOWER_LIM = lower_list[j]
        X_UPPER_LIM = upper_list[j]
        DIM = dim_list[j]

        
        results[i,j] = return_results(SOLVER_POPSIZE, problem_index, DIM, X_LOWER_LIM, X_UPPER_LIM, gen)


results = StandardScaler().fit_transform(results)

for column, label in zip(results.T, instance_list):
    print(column, np.array(generations))
    plt.plot(np.array(generations), column, label="F"+str(label))


plt.legend()
plt.show()

print(results)
