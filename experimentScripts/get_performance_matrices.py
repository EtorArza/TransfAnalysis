import subprocess
import nevergrad as ng
import numpy as np
import glob
import os

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




# folder_path = 'src/experiments/permus/instances/transfer_tsp_instances'
# extension = '.tsp'
# file_pattern = os.path.join(folder_path, f'*{extension}')
# files = sorted(glob.glob(file_pattern))
# lines = ["Instances,"+",".join([a+b for a in ["algo_swap","algo_exchange","algo_insert"] for b in ["randomrestart","halfrestart","quarterestart","sixthrestart"]])]
# for file_path in files:
#     print(f"Processing file: {file_path}")
#     instance = file_path.split("/")[-1]
#     res = subprocess.check_output(f"./main.out -local-search-permus {file_path}", shell=True, text=True)
#     res_list = eval(res)
#     lines.append(instance+","+",".join([str(el) for el in res_list]))
#     with open("experimentResults/problem_analisys/performanceMatrix_TSP.txt", "w") as file:
#         file.write("\n".join(lines))



# budget = 1000
# methods = ["RandomSearch", "DE", "PSO", "CMA", "EDA", "TwoPointsDE", "Powell", "BFGS", "RecES", "NelderMead","OnePlusOne","OnePlusLambda"]
# lines = ["Instances,"+ ",".join(["algo_"+ el for el in methods])]
# for i in range(1, 13):
#     line = f"_{i}_"
#     for method in methods:
#         print(f"Working on problem {i}, method {method}")
#         f = optimize(method, i, budget, None)
#         line += f",{f}"
#     lines.append(line)
#     with open("experimentResults/problem_analisys/performanceMatrix_continuous12.txt", "w") as file:
#         file.write("\n".join(lines))




# budget = 1000
# methods = ["RandomSearch", "DE", "PSO", "CMA", "EDA", "TwoPointsDE", "Powell", "BFGS", "RecES", "NelderMead","OnePlusOne","OnePlusLambda"]
# lines = ["Instances,"+ ",".join(["algo_"+ el for el in methods])]
# for NLO in [1,4,8,16,32,64]:
#     line = f"NLO_{NLO}"
#     for method in methods:
#         print(f"Working on problem NLO {NLO}, method {method}")
#         f = optimize(method, 0, budget, NLO)
#         line += f",{f}"
#     lines.append(line)
#     with open("experimentResults/problem_analisys/performanceMatrix_rokkonen.txt", "w") as file:
#         file.write("\n".join(lines))


import csv

def combine_files(featuresFilePath, performanceFilePath, output_path):
    # Read content from features file
    with open(featuresFilePath, 'r') as file1:
        file1_reader = csv.reader(file1)
        data1 = list(file1_reader)  # Read all rows into a list

    # Read content from performance file
    with open(performanceFilePath, 'r') as file2:
        file2_reader = csv.reader(file2)
        header2 = next(file2_reader)
        data2 = list(file2_reader)

    # Write combined content to the output file
    with open(output_path, 'w', newline='') as output_file:
        output_writer = csv.writer(output_file)

        # Write header
        header_content = ['Instances', 'Source'] + ['feature_f{}'.format(i) for i in range(1, len(data1[0]))] + header2[1:]
        print(header_content)
        output_writer.writerow(header_content)

        # Combine data and write rows
        for i in range(len(data1)):
            assert data1[i][0] == data2[i][0], f"{data1[i][0]} != {data2[i][0]}"
            output_writer.writerow([data1[i][0], 'none'] + data1[i][1:] + data2[i][1:])


# Combine resutlts for matilda analysis
combine_files("experimentResults/problem_analisys/featureMatrix_TSP_Matilda.txt",
              "experimentResults/problem_analisys/performanceMatrix_TSP.txt",
              "experimentResults/problem_analisys/Matilda_instanceSpaceData_tsp.txt")

combine_files("experimentResults/problem_analisys/featureMatrix_rokkonen_ELA.txt",
              "experimentResults/problem_analisys/performanceMatrix_rokkonen.txt",
              "experimentResults/problem_analisys/Matilda_instanceSpaceData_rokkonen.txt")

combine_files("experimentResults/problem_analisys/featureMatrix_continuous12_ELA.txt",
              "experimentResults/problem_analisys/performanceMatrix_continuous12.txt",
              "experimentResults/problem_analisys/Matilda_instanceSpaceData_continuous12.txt")

# Repeat and save files in matilda InstanceSpace folder
combine_files("experimentResults/problem_analisys/featureMatrix_TSP_Matilda.txt",
              "experimentResults/problem_analisys/performanceMatrix_TSP.txt",
              "other_src/InstanceSpace/trial_tsp/metadata.csv")

combine_files("experimentResults/problem_analisys/featureMatrix_rokkonen_ELA.txt",
              "experimentResults/problem_analisys/performanceMatrix_rokkonen.txt",
              "other_src/InstanceSpace/trial_rokkonen/metadata.csv")

combine_files("experimentResults/problem_analisys/featureMatrix_continuous12_ELA.txt",
              "experimentResults/problem_analisys/performanceMatrix_continuous12.txt",
              "other_src/InstanceSpace/trial_continuous12/metadata.csv")
