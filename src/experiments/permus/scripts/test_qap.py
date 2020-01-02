import os
import subprocess
import time
import random
from joblib import Parallel, delayed
from statistics import median, mean
import os


instances = [
    ["instances/qap/tai35a.dat.dat", 90.0, 2422002, 0.150],
    ["instances/qap/tai35b.dat.dat", 90.0, 283315445, 0.0],
    ["instances/qap/tai60a.dat.dat", 7.6 *60.0, 7205962, 1.098],
    ["instances/qap/tai60b.dat.dat", 7.6 *60.0 ,608215054, 0.0],
    ["instances/qap/tai100a.dat.dat",36.1*60.0, 21052466, 1.091],
    ["instances/qap/tai100b.dat.dat",36.1*60.0, 1185996137, 0.039],
    ["instances/qap/lipa40a.dat.dat",2.3 *60.0, 31538, 0.0],
    ["instances/qap/lipa40b.dat.dat",2.3 *60.0, 476581, 0.0],
    ["instances/qap/sko49.dat.dat", 4.1*60.0, 23386, 0.034],
]


controller_directory = "experimentation_results/qap_incremental_to_10s/controllers"



BINARY_PATH = "neat"
def _prep_cmd(instance_path, max_time, controller_path):

    cmd = "./"
    cmd += BINARY_PATH + ' '
    cmd += "test" + ' '
    cmd += "qap" + ' '
    cmd += instance_path + ' ' # instance path
    cmd += controller_path + ' '
    cmd += str(max_time) + ' ' # time

    return cmd


N_REPS = 1
def print_row(instance_and_params, controller_path):
        cmd = _prep_cmd(instance_and_params[0], instance_and_params[1], controller_path)
        best = instance_and_params[2]
        ardp_ours = []
        time.sleep(random.random()*5.0)
        for _ in range(N_REPS):
            ardp_ours.append((abs(int(subprocess.check_output(cmd, shell=True))) - best) / best * 100)
        print(instance_and_params[0].split("/")[-1], controller_path, median(ardp_ours), instance_and_params[-1])


controller_paths = []
directory = os.fsencode(controller_directory)
for file in os.listdir(directory):
    filename = os.fsdecode(file)
    if True:# filename.endswith(".asm") or filename.endswith(".py"): 
        controller_paths.append(os.path.join(controller_directory, filename))
        continue
    else:
        continue




# Test best controller on all instances
N_REPS = 10
controller_path = controller_directory + "/" + "fittest_1400"
Parallel(n_jobs=7, verbose=0, backend="multiprocessing")(delayed(print_row)(instance, controller_path) for instance in instances)
print("----")


# # Test all controllers
# N_REPS = 900
# instances[0][1] = 10.0
# Parallel(n_jobs=7, verbose=0, backend="multiprocessing")(delayed(print_row)(instances[0], controller) for controller in controller_paths)
