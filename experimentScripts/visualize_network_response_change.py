import numpy as np
from matplotlib import pyplot as plt
import subprocess
from tqdm import tqdm as tqdm
import pandas as pd

result_file_path = "experimentResults/visualize_network_response_change/figures"
controller_path_folder = "experimentResults/visualize_network_response_change/controllers/top_controllers"
res_csv_file = "experimentResults/visualize_network_response_change/comparison_diff_params.csv"
subprocess.run(f"mkdir -p {result_file_path}",shell=True)

# rm log.txt -f && cd ~/Dropbox/BCAM/02_NEAT_transferability/code/NEAT_code/ && make &&  cd .. && rsync -av --exclude=".*" NEAT_code /dev/shm/ && cd /dev/shm/NEAT_code && python experimentScripts/visualize_network_response_change.py

SOLVER_POPSIZE=8
MAX_SOLVER_FE=400
N_EVALS=2000
PROBLEM_DIM=20
THREADS=1

PROBLEM_INDEX=1


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

    for controller_name in tqdm(controller_filename_list):
        controller_path = controller_path_folder + "/" + controller_name
        solver_popsize = int(controller_name.split("SOLVERPOPSIZE_")[-1].split("_")[0])
        max_solver_fe = int(controller_name.split("MAXSOLVERFE_")[-1].split("_")[0])
        run_with_controller_continuous(PROBLEM_INDEX, solver_popsize, max_solver_fe, controller_path)
        plot_from_detailed_response(f"problem_{PROBLEM_INDEX}_popsize_{SOLVER_POPSIZE}_maxsolverfe_{MAX_SOLVER_FE}")
        subprocess.run("rm -f responses.txt",shell=True)
        subprocess.run("rm -f responses.txt",shell=True)    
        subprocess.run("rm -f responses.txt",shell=True)
    
    df = pd.DataFrame()
    for i in tqdm(range(8)):
        max_solver_fe_controller = [400, 6400, 400, 6400, 1600, 1600, 1600, 1600][i]
        max_solver_fe_problem =    [400, 400, 6400, 6400, 1600, 1600, 1600, 1600][i]
        popsize_controller =       [16, 16, 16, 16, 8, 128,8, 128][i]
        popsize_problem =          [16, 16, 16, 16, 8, 8, 128, 128][i]
        controller_name = f"TrainOnlyInF_1_seed2_MAXSOLVERFE_{max_solver_fe_controller}_SOLVERPOPSIZE_{popsize_controller}_best.controller"
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
