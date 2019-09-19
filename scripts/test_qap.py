import os
import subprocess
from joblib import Parallel, delayed
from statistics import mean


instances = [
    ["/home/paran/Dropbox/BCAM/QAPinstances/tai35a.dat.dat", 90.0, 2422002, 0.150],
    ["/home/paran/Dropbox/BCAM/QAPinstances/tai35b.dat.dat", 90.0, 283315445, 0.0],
    ["/home/paran/Dropbox/BCAM/QAPinstances/tai60a.dat.dat", 7.6 *60.0, 7205962, 1.098],
    ["/home/paran/Dropbox/BCAM/QAPinstances/tai60b.dat.dat", 7.6 *60.0 ,608215054, 0.0],
    ["/home/paran/Dropbox/BCAM/QAPinstances/tai100a.dat.dat",36.1*60.0, 21052466, 1.091],
    ["/home/paran/Dropbox/BCAM/QAPinstances/tai100b.dat.dat",36.1*60.0, 1185996137, 0.039],
    ["/home/paran/Dropbox/BCAM/QAPinstances/lipa40a.dat.dat",2.3 *60.0, 31538, 0.0],
    ["/home/paran/Dropbox/BCAM/QAPinstances/lipa40b.dat.dat",2.3 *60.0, 476581, 0.0],
    ["/home/paran/Dropbox/BCAM/QAPinstances/sko49.dat.dat", 4.1*60.0, 23386, 0.034],
]


BINARY_PATH = "neat"
CONTROLLER_PATH = "fittest_qap_1"
def _prep_cmd(instance_path, max_time):

    cmd = "./"
    cmd += BINARY_PATH + ' '
    cmd += "test" + ' '
    cmd += "qap" + ' '
    cmd += instance_path + ' ' # instance path
    cmd += CONTROLLER_PATH + ' '
    cmd += str(max_time) + ' ' # time

    return cmd

N_REPS = 20
def print_row(instance_and_params):
        cmd = _prep_cmd(instance_and_params[0], instance_and_params[1])
        best = instance_and_params[2]
        ardp_ours = []
        for _ in range(N_REPS):
            ardp_ours.append((abs(int(subprocess.check_output(cmd, shell=True))) - best) / best * 100)
        print(instance_and_params[0].split("/")[-1], mean(ardp_ours), instance_and_params[-1])


Parallel(n_jobs=2)(delayed(print_row)(instance_and_params) for instance_and_params in instances)



