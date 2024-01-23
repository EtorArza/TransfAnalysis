import subprocess
import nevergrad as ng
import numpy as np

def update_parameter_in_file(file_path, new_nlo):
    with open(file_path, 'r') as file:
        file_content = file.readlines()
    file_content[6] = str(new_nlo) + '\n'
    with open(file_path, 'w') as file:
        file.writelines(file_content)




def f_continuous(x, problem_index, NLO):
    x_str = ",".join(map(str, x))

    assert bool(problem_index==0) != bool(NLO is None)
    if problem_index==0:
        update_parameter_in_file("src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/quad_function.dat", NLO)

    cmd = f"./main.out -evaluate-continuous-problem {problem_index} {x_str}"
    res = subprocess.check_output(cmd, shell=True, text=True)
    return float(res)




def my_function(x):
    return abs(sum(x - 1))

def print_candidate_and_value(optimizer, candidate, value):
    print(candidate, value)




def optimize(method, problem_index, budget, NLO=None):

    assert method in ng.optimizers.registry.keys()
    dim = 20
    instrum = ng.p.Instrumentation(ng.p.Array(shape=(dim,),lower=0.0,  upper=1.0))
    # instrum.value = np.random.random(dim)
    optimizer = ng.optimizers.registry[method](parametrization=instrum, budget=budget, num_workers=1)

    for _ in range(optimizer.budget):
        x = optimizer.ask()
        loss = -f_continuous(x.value[0][0], problem_index, NLO)
        optimizer.tell(x, loss)

    recommendation = optimizer.provide_recommendation()
    return f_continuous(recommendation.value[0][0], problem_index, NLO)







budget = 1000
methods = ["RandomSearch", "DE", "PSO", "CMA"]
lines = ["Instances,"+ ",".join(["algo_"+ el for el in methods])]
for i in range(1, 13):
    line = f"F_{i}"
    for method in methods:
        print(f"Working on problem {i}, method {method}")
        f = optimize(method, i, budget, None)
        line += f",{f}"
    lines.append(line)
    with open("experimentResults/problem_analisys/performanceMatrix_continuous12.txt", "w") as file:
        file.write("\n".join(lines))




budget = 1000
methods = ["RandomSearch", "DE", "PSO", "CMA"]
lines = ["Instances,"+ ",".join(["algo_"+ el for el in methods])]
for NLO in [1,4,8,16,32,64]:
    line = f"NLO_{NLO}"
    for method in methods:
        print(f"Working on problem NLO {NLO}, method {method}")
        f = optimize(method, 0, budget, NLO)
        line += f",{f}"
    lines.append(line)
    with open("experimentResults/problem_analisys/performanceMatrix_rokkonen.txt", "w") as file:
        file.write("\n".join(lines))
