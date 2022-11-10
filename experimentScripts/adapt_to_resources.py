import numpy as np
from matplotlib import pyplot as plt
import subprocess
from tqdm import tqdm as tqdm
import pandas as pd

result_file_path = "experimentResults/adapt_to_resources/figures"
controller_path_folder = "experimentResults/adapt_to_resources/controllers/top_controllers"
res_csv_file = "experimentResults/adapt_to_resources/comparison_diff_params.csv"
subprocess.run(f"mkdir -p {result_file_path}",shell=True)

# rm log.txt -f && cd ~/Dropbox/BCAM/02_NEAT_transferability/code/NEAT_code/ && make &&  cd .. && rsync -av --exclude=".*" NEAT_code /dev/shm/ && cd /dev/shm/NEAT_code && python experimentScripts/adapt_to_resources.py

SOLVER_POPSIZE=16
N_EVALS=2000
PROBLEM_DIM=12


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


def write_conf_file_continuous(problem_index, solver_popsize, max_solver_fe, controller_path, print_positions):
    tmp_ini_file_string = f"""
[Global] 
mode = test
PROBLEM_NAME = real_func

THREADS = 1
N_EVALS = {N_EVALS}
N_REPS = 1
CONTROLLER_PATH = {controller_path}
SEED = 2

PRINT_POSITIONS = {print_positions}
FULL_MODEL = false
COMPUTE_RESPONSE = true
 

SOLVER_POPSIZE = {solver_popsize}


PROBLEM_INDEX = {problem_index}
PROBLEM_DIM = {PROBLEM_DIM}
MAX_SOLVER_FE = {max_solver_fe}

"""


    with open("tmp_conf_file.ini","w") as f:
        f.writelines(tmp_ini_file_string)

def run_with_controller_continuous(problem_index, solver_popsize, max_solver_fe, controller_path, print_positions="false"):


    write_conf_file_continuous(problem_index, solver_popsize, max_solver_fe, controller_path, print_positions)
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
    print("Instance list:\n", "\n".join(controller_filename_list))

    
    df = pd.DataFrame()
    for i in tqdm(range(8)):
        max_solver_fe_controller = [400, 6400, 400, 6400, ][i]
        max_solver_fe_problem =    [400, 400, 6400, 6400, ][i]
        popsize_controller =       [16, 16, 16, 16,       ][i]
        popsize_problem =          [16, 16, 16, 16,       ][i]
        controller_name = f"TrainOnlyInF_1_seed2_MAXSOLVERFE_{max_solver_fe_controller}_best.controller"
        controller_path = controller_path_folder + "/" + controller_name
        for problem_index in range(1,13):
            score = run_with_controller_continuous(problem_index, popsize_problem, max_solver_fe_problem, controller_path)
            df = df.append({
                "problem_index": problem_index,
                "max_solver_fe_controller": max_solver_fe_controller,
                "max_solver_fe_problem": max_solver_fe_problem, 
                "popsize_controller": popsize_controller, 
                "popsize_problem": popsize_problem, 
                "score": score,
            }, ignore_index=True)
    
    # # For debugging python with interactive shell. Start interactive shell.
    # import code
    # code.interact(local=locals())
    df.to_csv(res_csv_file,  index=False)
