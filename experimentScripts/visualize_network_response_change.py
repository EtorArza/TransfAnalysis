import numpy as np
from matplotlib import pyplot as plt
import subprocess


result_file_path = "experimentResults/visualize_network_response_change/"
subprocess.run(f"mkdir -p {result_file_path}",shell=True)

# rm log.txt -f && cd ~/Dropbox/BCAM/02_NEAT_transferability/code/NEAT_code/ && make &&  cd .. && rsync -av --exclude=".*" NEAT_code /dev/shm/ && cd /dev/shm/NEAT_code && python experimentScripts/visualize_network_response_change.py

SOLVER_POPSIZE=10
MAX_SOLVER_FE=1000
N_EVALS=20
N_EVALS_TEST=100
PROBLEM_DIM=20
THREADS=1




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
        plt.ylim((np.min(data), np.max(data)))
        plt.legend()
        plt.tight_layout()
        plt.savefig(result_file_path + savefile_prefix + "_" + x_label + "_detailedoutput.pdf")
        plt.close()


def write_conf_file_continuous(PROBLEM_INDEX,CONTROLLER_PATH, PRINT_POSITIONS):

    tmp_ini_file_string = f"""
[Global] 
mode = test
PROBLEM_NAME = real_func

THREADS = 1
N_EVALS = {N_EVALS}
N_REPS = 1
CONTROLLER_PATH = {CONTROLLER_PATH}
SEED = 2

PRINT_POSITIONS = {PRINT_POSITIONS}
FULL_MODEL = false
COMPUTE_RESPONSE = true
 

SOLVER_POPSIZE = {SOLVER_POPSIZE}


PROBLEM_INDEX = {PROBLEM_INDEX}
PROBLEM_DIM = {PROBLEM_DIM}
MAX_SOLVER_FE = {MAX_SOLVER_FE}

"""


    with open("tmp_conf_file.ini","w") as f:
        f.writelines(tmp_ini_file_string)

def run_with_controller_continuous(PROBLEM_INDEX, CONTROLLER_PATH, PRINT_POSITIONS="false"):


    write_conf_file_continuous(PROBLEM_INDEX,CONTROLLER_PATH,PRINT_POSITIONS)
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
    
    controller_path = "experimentResults/transfer_16_continuous_problems/controllers/top_controllers/TrainOnlyInF_1_best.controller"
    PROBLEM_INDEX = 1

    run_with_controller_continuous(PROBLEM_INDEX, controller_path)
    plot_from_detailed_response(f"problem_{PROBLEM_INDEX}")
    subprocess.run("rm -f responses.txt",shell=True)    
