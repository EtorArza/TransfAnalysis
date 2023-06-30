import os
from matplotlib import pyplot as plt

directory_of_controller_directories = "src/experiments/permus/results/overfitting/test_controllers/"
instance = "src/experiments/permus/instances/qap/cut_instances/cut60_tai80b"
cwd = "/home/paran/Dropbox/BCAM/02_NEAT_transferability/code"
exp_dir = "src/experiments/permus/results/overfitting/"

CONF_FILE = """

[Global] 
MODE = test ;
PROBLEM_NAME = permu ;


THREADS = 7 ;
N_EVALS = 10 ;
N_REPS = 1 ;
CONTROLLER_PATH = $3 ; 
COMPUTE_RESPONSE = false ;

MAX_SOLVER_TIME = 0.25 ; 
POPSIZE = 20 ;


PROBLEM_TYPE = qap ; 
PROBLEM_PATH = $2 ; 


"""

for controller_directory in os.listdir(directory_of_controller_directories):
    os.system("touch score.txt && rm score.txt")
    label = controller_directory
    controller_directory = directory_of_controller_directories + controller_directory + "/"
    print(controller_directory)
    for controller in os.listdir(controller_directory):
        conf_file_this_round = CONF_FILE.replace("$3", controller_directory + controller).replace("$2", instance)
        with open("tmp.ini", "w") as f:
            print(conf_file_this_round, file=f)
        os.chdir(cwd)
        os.system("ls")
        os.system("./main.out tmp.ini")
        
    lines = []
    with open("score.txt", "r") as f:
        for line in f:
            print(line)
            lines += [eval(line.strip("\n"))]

    print(lines)
    lines.sort(key= lambda x: x[2])

    x = []
    y = []
    for line in lines:
        y.append(line[0][0])
        x.append(int(line[2].split("gen_")[-1]))

    plt.plot(x, y, label=label)

plt.legend()
plt.savefig(exp_dir+"result.pdf")
plt.close()

