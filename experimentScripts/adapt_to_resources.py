import numpy as np
from matplotlib import pyplot as plt
import subprocess
from tqdm import tqdm as tqdm
import pandas as pd

result_file_path = "experimentResults/adapt_to_resources/figures"
controller_path_folder = "experimentResults/adapt_to_resources/controllers/top_controllers"
subprocess.run(f"mkdir -p {result_file_path}",shell=True)

# rm log.txt -f && cd ~/Dropbox/BCAM/02_NEAT_transferability/code/NEAT_code/ && make &&  cd .. && rsync -av --exclude=".*" NEAT_code /dev/shm/ && cd /dev/shm/NEAT_code && python experimentScripts/adapt_to_resources.py

SOLVER_POPSIZE=8
PROBLEM_DIM=12
N_EVALS_TEST=2000

solver_evals = [400, 1200]

def read_matrices(file_path):
    result = []
    with open(file_path) as f:
        for line in f:
            line = line.strip()
            if "i" not in line:
                continue
            i = int(line.split(" = ")[-1].split(" ( ")[0])
            if i==0:
                result.append([])
        
            values = line.split("( ")[-1].split(" )")[0].split(" ")
            values = [float(el) for el in values]
            result[-1].append(values)
    for i, _ in enumerate(result):
        result[i] = np.array(result[i])
    return result


def plot_from_detailed_response(savefile_prefix):

    x_labels = ["solution rank", "relative time"]

    datasets = read_matrices("detailed_response.txt")


    for i, data in enumerate(datasets):
        x_label = x_labels[i]
        # When applying the network output, we multiply it by 2, so that the parameters are in the interval (-2,2)
        # This means that the pso params applied by the hyperheuristic are the network out * 2.
        data = data * 2 

        plt.plot(np.array(range(1, data.shape[0]+1)), data[:,0], label="Momentum")
        plt.plot(np.array(range(1, data.shape[0]+1)), data[:,1], label="GBest")
        plt.plot(np.array(range(1, data.shape[0]+1)), data[:,2], label="LBest")
        plt.xlabel(x_label)
        plt.ylim((-2.1,2.1))
        plt.legend()
        plt.tight_layout()
        plt.savefig(result_file_path + "/" + savefile_prefix + "_" + x_label.replace(" ", "_") + "_detailedoutput.pdf")
        plt.close()


def write_conf_file_continuous(problem_index, solver_popsize, max_solver_fe, controller_path):
    tmp_ini_file_string = f"""
[Global] 
mode = test
PROBLEM_NAME = real_func

THREADS = 1
N_EVALS = {N_EVALS_TEST}
N_REPS = 1
CONTROLLER_PATH = {controller_path}
SEED = 2

PRINT_POSITIONS = false
FULL_MODEL = false
COMPUTE_RESPONSE = false
 

SOLVER_POPSIZE = {solver_popsize}


PROBLEM_INDEX = {problem_index}
PROBLEM_DIM = {PROBLEM_DIM}
MAX_SOLVER_FE = {max_solver_fe}

"""


    with open("tmp_conf_file.ini","w") as f:
        f.writelines(tmp_ini_file_string)

        

def write_conf_file_permus(problem_path, solver_popsize, max_solver_fe, controller_path):
    tmp_ini_file_string = f"""
[Global] 
MODE = test
PROBLEM_NAME = permu


THREADS = 1
CONTROLLER_PATH = {controller_path}
COMPUTE_RESPONSE = false
N_REPS = 1
N_EVALS = {N_EVALS_TEST}


MAX_SOLVER_FE = {max_solver_fe}

PROBLEM_TYPE = {problem_path.split(".")[-1]}
PROBLEM_PATH = {problem_path}

SOLVER_POPSIZE = {solver_popsize}

"""


    with open("tmp_conf_file.ini","w") as f:
        f.writelines(tmp_ini_file_string)



def run_with_controller(problem_domain, problem_index_or_path, solver_popsize, max_solver_fe, controller_path):

    if problem_domain=="continuous":
        write_conf_file_continuous(problem_index_or_path, solver_popsize, max_solver_fe, controller_path)

    elif problem_domain=="permus":
        write_conf_file_permus(problem_index_or_path, solver_popsize, max_solver_fe, controller_path)

    
    subprocess.run("rm -f detailed_response.txt" , shell=True) 
    subprocess.run("rm -f positions.txt" , shell=True) 
    subprocess.run("./neat tmp_conf_file.ini > /dev/null", shell=True) # omit out
    with open("score.txt","r") as f:
        res = f.readline().strip()
    score = res.split("]")[0].strip("[")
    subprocess.run("rm -f responses.txt",shell=True)    
    subprocess.run("rm -f score.txt",shell=True)
    #subprocess.run("rm tmp_conf_file.ini",shell=True)
    return score

if __name__ == "__main__":
    
    import os
    controller_filename_list = [f for f in os.listdir("./"+controller_path_folder)]
    print("Controller list:\n", "\n".join(controller_filename_list))

    # res1 = run_with_controller("continuous",1,8,400,"experimentResults/adapt_to_resources/controllers/top_controllers/TrainOnlyInF_1_seed2_MAXSOLVERFE_400_best.controller")
    # res2 =run_with_controller("permus","src/experiments/permus/instances/transfer_permuproblems/lop/N-t65d11xx_150cut.lop",8,400,"experimentResults/adapt_to_resources/controllers/top_controllers/TrainOnlyInF_1_seed2_MAXSOLVERFE_400_best.controller")
    
    def listdir_fullpath(d):
        return [os.path.join(d, f) for f in os.listdir(d)]

    def flatten(l):
        return [item for sublist in l for item in sublist]

    for problemset_idx, controller_prefix, problem_idx_or_path in zip(
            range(4), 
            ["TrainOnlyInF__seed2",None,"Permus_seed2", "Qap_seed2"],
            [   list(range(1,13)), 
                None,
                flatten([listdir_fullpath(f"src/experiments/permus/instances/transfer_permuproblems/{prob}/") for prob in ["lop","pfsp","qap","tsp"]]),
                listdir_fullpath("src/experiments/permus/instances/transfer_qap_cut_instances/"),
            ]        
        ):

        if controller_prefix == None:
            continue

        res_csv_file = f"experimentResults/adapt_to_resources/comparison_diff_params_{controller_prefix}.csv"

        df = pd.DataFrame()
        for i in tqdm(range(4)):
            max_solver_fe_controller = [solver_evals[0], solver_evals[1], solver_evals[0], solver_evals[1]][i]
            max_solver_fe_problem =    [solver_evals[0], solver_evals[0], solver_evals[1], solver_evals[1]][i]
            popsize_problem = 8
            controller_name = controller_prefix + f"_MAXSOLVERFE_{max_solver_fe_controller}_best.controller"
            controller_path = controller_path_folder + "/" + controller_name
            for problem_index in problem_idx_or_path:
                score = run_with_controller("continuous" if problemset_idx<2 else "permus", problem_index, popsize_problem, max_solver_fe_problem, controller_path)
                df = df.append({
                    "problem_index": problem_index,
                    "max_solver_fe_controller": max_solver_fe_controller,
                    "max_solver_fe_problem": max_solver_fe_problem, 
                    "popsize_problem": popsize_problem, 
                    "score": float(score),
                }, ignore_index=True)
        
        # # For debugging python with interactive shell. Start interactive shell.
        # import code; code.interact(local=locals())

        print("---")
        print(controller_prefix, len(problem_idx_or_path))
        from statistics import mean
        for evs in solver_evals:
            rows = df.iloc[df.query(f"max_solver_fe_problem == {evs}").groupby(["problem_index"])['score'].idxmax()]
            print(evs, "-> ", mean(rows.max_solver_fe_controller == evs))


        
        df.to_csv(res_csv_file,  index=False)
