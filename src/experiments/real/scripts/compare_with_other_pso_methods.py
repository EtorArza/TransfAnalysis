import pandas as pd
import subprocess

CONTROLLER_PATH = "src/experiments/real/results/comparison_other_pso/controllers_trained_on_all_problems/top_controllers/"
N_EVALS = 30
THREADS = 7


def write_conf_file(METHOD_SHORTNAME,SOLVER_POPSIZE,PROBLEM_INDEX,PROBLEM_DIM,MAX_SOLVER_FE,X_LOWER_LIM,X_UPPER_LIM,F_COMPETITION, N_REPS):

    values = {
        "SOLVER_POPSIZE":SOLVER_POPSIZE,
        "PROBLEM_INDEX":PROBLEM_INDEX,
        "PROBLEM_DIM":PROBLEM_DIM,
        "MAX_SOLVER_FE":MAX_SOLVER_FE,
        "X_LOWER_LIM":X_LOWER_LIM,
        "X_UPPER_LIM":X_UPPER_LIM,
        "CONTROLLER_PATH":CONTROLLER_PATH,
        "N_EVALS":N_EVALS,
        "N_REPS":N_REPS,
        "THREADS":THREADS,
    }

    tmp_ini_file_string = """
[Global] 
mode = test
PROBLEM_NAME = real_func

THREADS = {THREADS}
N_EVALS = {N_EVALS}
N_REPS = {N_REPS}
;CONTROLLER_PATH = {CONTROLLER_PATH}trained_with_all_problems_best.controller

;CONTROLLER_PATH = {CONTROLLER_PATH}LeaveOutF_{PROBLEM_INDEX}_best.controller
;CONTROLLER_PATH = {CONTROLLER_PATH}TrainOnlyInF_{PROBLEM_INDEX}_best.controller
;CONTROLLER_PATH = {CONTROLLER_PATH}trained_with_all_problems_false_best.controller
CONTROLLER_PATH = {CONTROLLER_PATH}trained_with_all_problems_true_best.controller

PRINT_POSITIONS = false
FULL_MODEL = true

SOLVER_POPSIZE = {SOLVER_POPSIZE}


PROBLEM_INDEX = {PROBLEM_INDEX}
PROBLEM_DIM = {PROBLEM_DIM}
MAX_SOLVER_FE = {MAX_SOLVER_FE}

X_LOWER_LIM = {X_LOWER_LIM}
X_UPPER_LIM = {X_UPPER_LIM}
""".format(**values)

    with open("tmp_conf_file.ini","w") as f:
        f.writelines(tmp_ini_file_string)
    
results = []
def record_results(METHOD_SHORTNAME,SOLVER_POPSIZE,PROBLEM_INDEX,PROBLEM_DIM,MAX_SOLVER_FE,X_LOWER_LIM,X_UPPER_LIM,F_COMPETITION, N_REPS):
    write_conf_file(METHOD_SHORTNAME, SOLVER_POPSIZE, PROBLEM_INDEX,
                    PROBLEM_DIM, MAX_SOLVER_FE, X_LOWER_LIM, X_UPPER_LIM, F_COMPETITION, N_REPS)
    subprocess.run("./neat tmp_conf_file.ini > /dev/null",shell=True)
    # subprocess.run("./neat tmp_conf_file.ini",shell=True)
    with open("result.txt","r") as f:
        res = f.readline().strip()
    res_list = eval("["+str(F_COMPETITION)+","+res+"]") # file="coparison_result.txt")
    print(METHOD_SHORTNAME, MAX_SOLVER_FE, SOLVER_POPSIZE, (X_LOWER_LIM,X_UPPER_LIM), "{:.4e}".format(res_list[0]), "{:.4e}".format(-res_list[1][0][0]), res_list[1][1], PROBLEM_DIM, N_REPS, sep=",")
    results.append([METHOD_SHORTNAME, MAX_SOLVER_FE, SOLVER_POPSIZE, (X_LOWER_LIM,X_UPPER_LIM), "{:.4e}".format(res_list[0]), "{:.4e}".format(-res_list[1][0][0]), res_list[1][1], PROBLEM_DIM])
    subprocess.run("rm result.txt",shell=True)


def string_number_to_tex_format(string_number_in_scientific_notation):
    pieces = string_number_in_scientific_notation.split("e")
    exponent = str(abs(int(pieces[1])))
    sign = "+" if int(pieces[1]) >= 0 else "-"
    return "$" + pieces[0] + "\cdot 10^{" + sign + exponent + "}$"

def get_two_numbers_to_text_highlight_smallest(string_number_in_scientific_notation1, string_number_in_scientific_notation2):
    num1 = float(string_number_in_scientific_notation1)
    num2 = float(string_number_in_scientific_notation2)

    comp = ""
    if num1 < num2:
        comp = "$<$"
    elif num1 >  num2:
        comp = "$>$"
    else:
        comp = "$=$"
    
    string_number_in_scientific_notation1 =  string_number_to_tex_format(string_number_in_scientific_notation1)
    string_number_in_scientific_notation2 =  string_number_to_tex_format(string_number_in_scientific_notation2)

    return string_number_in_scientific_notation1 + " & " + comp + " & " + string_number_in_scientific_notation2

df = pd.read_csv("src/experiments/real/results/comparison_other_pso/comparison_other_pso_methods.csv")

print("other_method_name,FE,swarm_size,bounds,f_other_method,f_proposed_method,test_function_index,dim")

METHOD_SHORTNAME = ""
for row_index, row in df.iterrows():
    
    
    if str(row[0]).count("|") == 1: # new method
        problem_lims_row_index = row_index
        METHOD_SHORTNAME = row[0].split("|")[-1]
    
    elif str(row[0]).count("|") == 3: # new params, execute algorithm
        PROBLEM_DIM = int(row[0].split("|")[0])
        MAX_SOLVER_FE = int(row[0].split("|")[1])
        SOLVER_POPSIZE = int(row[0].split("|")[2])
        N_REPS = 1 #int(row[0].split("|")[3])
        for col_index in range(1,len(row)):
            PROBLEM_INDEX = col_index # first problem is in col 1, and the problem index is 1 too.
            if  df.isnull().iloc[(problem_lims_row_index,PROBLEM_INDEX)]:
                continue
            X_LOWER_LIM = float(df.iloc[(problem_lims_row_index,PROBLEM_INDEX)].split("|")[0])
            X_UPPER_LIM = float(df.iloc[(problem_lims_row_index,PROBLEM_INDEX)].split("|")[1])
            F_COMPETITION = float(df.iloc[(row_index,PROBLEM_INDEX)])

            record_results(METHOD_SHORTNAME,SOLVER_POPSIZE,PROBLEM_INDEX,PROBLEM_DIM,MAX_SOLVER_FE,X_LOWER_LIM,X_UPPER_LIM,F_COMPETITION, N_REPS)
    

for i in range(len(results)):
    results[i][4] = str(results[i][4])
    results[i][5] = str(results[i][5])



list_of_tex_friendly_rows = ["  &  ".join(['F'+str(el[6]),get_two_numbers_to_text_highlight_smallest(el[4],el[5])])+r" \\" for el in results]

print_color = False
for line in list_of_tex_friendly_rows:
    print_color = not print_color
    if print_color:
        print(r"\rowcolor{LightCyan}")
    print(line)