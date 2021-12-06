import pandas as pd
import subprocess


def write_conf_file_continuous(PROBLEM_PATH,CONTROLLER_PATH, n_evals, seed):

    tmp_ini_file_string = f"""
[Global] 
MODE = test
PROBLEM_NAME = permu

THREADS = 6
N_EVALS = {n_evals}
N_REPS = 1
CONTROLLER_PATH = {CONTROLLER_PATH}
COMPUTE_RESPONSE = false


MAX_SOLVER_FE = 3000 ; Max time the controller has to solve the permutation problem.

PROBLEM_TYPE = tsp
PROBLEM_PATH = {PROBLEM_PATH}

SEED = {seed}
"""

    with open("tmp_conf_file.ini","w") as f:
        f.writelines(tmp_ini_file_string)




controller_path = "src/experiments/permus/instances/tsp_instances/kroA100.tsp_best.controller"
csv_path = "src/experiments/permus/instances/tsp_instances/comparison_wu_et_al.csv"

df = pd.read_csv(csv_path)
problem_names = df["Problem Name"]

problem_paths = ["src/experiments/permus/instances/tsp_instances/" + el + ".tsp" for el in problem_names]

for problem_path, problem_name, other_method_score in zip(problem_paths, problem_names, df["Wu et al."]):
    write_conf_file_continuous(problem_path, controller_path, 1, 2)
    subprocess.run("rm -f score.txt",shell=True)
    subprocess.run("./neat tmp_conf_file.ini > /dev/null", shell=True) # omit out
    with open("score.txt","r") as f:
        res = f.readline().strip()
    score = res.split("]")[0].strip("[")
    score = abs(round(float(score)))
    subprocess.run("rm tmp_conf_file.ini",shell=True)
    print(problem_name, score, other_method_score, abs(float(score)) < abs(other_method_score), sep=", ")

