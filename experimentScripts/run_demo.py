from tqdm import tqdm as tqdm

problem_list = [
"demo/instances/kroA100cut50.tsp",
"demo/instances/kroB100cut50.tsp",
"demo/instances/kroC100cut50.tsp",
"demo/instances/pr107cut50.tsp",
"demo/instances/pr124cut50.tsp",
"demo/instances/pr136cut50.tsp",
]

def train_in_problem(train_problem_path, train_seed):

    train_config_content=f"""
[Global] 

MODE = train
PROBLEM_NAME = permu_multi


MAX_TRAIN_ITERATIONS = 2000
MAX_TRAIN_TIME = 600
POPSIZE = 1000
THREADS = 6

SEARCH_TYPE = phased
SEED = {train_seed}

CONTROLLER_NAME_PREFIX = demo{train_problem_path.split("/")[-1]}_{train_seed}
EXPERIMENT_FOLDER_NAME = demo

MAX_SOLVER_FE = 400
SOLVER_POPSIZE = 8


PROBLEM_TYPE = tsp
PROBLEM_PATH = dummy_path
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS  = {train_problem_path}
"""
    with open("demo/train_local.ini", 'w') as file: 
        file.write(train_config_content)
    import subprocess
    subprocess.run(['./main.out', 'demo/train_local.ini'],  stdout=subprocess.DEVNULL)


def test(train_problem_path, train_seed, test_problem_path):
    test_config_content=f"""
[Global] 
MODE = test
PROBLEM_NAME = permu

THREADS = 1
N_EVALS = 10000
N_REPS = 1
CONTROLLER_PATH = demo/top_controllers/demo{train_problem_path.split("/")[-1]}_{train_seed}_best.controller
COMPUTE_RESPONSE = true


MAX_SOLVER_FE = 400
SOLVER_POPSIZE = 8


PROBLEM_TYPE = tsp
PROBLEM_PATH = {test_problem_path}


"""
    with open("demo/test_local.ini", 'w') as file: 
        file.write(test_config_content)
    import subprocess
    subprocess.run(['./main.out', 'demo/test_local.ini'],  stdout=subprocess.DEVNULL)


# Get the analysis low level data
pb = tqdm(total=10*len(problem_list)*len(problem_list))
for seed in range(2,12):
    for train_problem in problem_list:
        train_in_problem(train_problem, seed)
        for test_problem in problem_list:
            test(train_problem, seed, test_problem)
            pb.update()