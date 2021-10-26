from statistics import mean, variance, stdev, median
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from seaborn import clustermap
import seaborn as sns
from scipy.ndimage.filters import gaussian_filter
from copy import deepcopy
import random
import fnmatch
import matplotlib

# save_fig_path = "/home/paran/Dropbox/BCAM/02_NEAT_permus/paper/images/permu_problems_transfer/"

save_fig_paths = [
#"/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus_multi/results/qap_cut_multi_vs_mono/score_multi_instance_cut_qap.txt",
"experimentResults/transfer_permus_qap/results/figures/",
"experimentResults/transfer_permus_qap/results/figures/",
"experimentResults/transfer_permus_qap/results/figures/",
"experimentResults/transfer_permus_problems/results/figures/",
"experimentResults/transfer_permus_problems/results/figures/",
"experimentResults/transfer_permus_problems/results/figures/",
"experimentResults/transfer_16_continuous_problems/results/figures/",
"experimentResults/transfer_16_continuous_problems/results/figures/"
]


txt_paths = [
#"src/experiments/permus_multi/results/qap_cut_multi_vs_mono/score_multi_instance_cut_qap.txt",
"src/experiments/permus/results/4by4_permu_problems/result_score_transfer_permuproblem_0_1s_2h.txt",
"src/experiments/permus/results/4by4_permu_problems/result_score_transfer_permuproblem_0_25s_1h.txt",
"src/experiments/permus/results/4by4_permu_problems/result_score_transfer_permuproblem_0_1s_12h.txt",
"src/experiments/permus/results/transfer_qap_with_cut_instances/result_score_transfer_qap_0_1s_2h.txt",
"src/experiments/permus/results/transfer_qap_with_cut_instances/result_score_transfer_qap_0_25s_1h.txt",
"src/experiments/permus/results/transfer_qap_with_cut_instances/result_score_transfer_qap_0_1s_12h.txt",
"experimentResults/transfer_16_continuous_problems/results/score.txt",
"experimentResults/transfer_16_continuous_problems/results/score.txt",
]


transfer_exp_list =[
#"QAP_MULTI",
"PERMUPROB",
"PERMUPROB",
"PERMUPROB",
"QAP",
"QAP",
"QAP",
"LOO16",
"Transfer16OnlyOne",
]

i = 0
for input_txt, transfer_exp, save_fig_path in zip(txt_paths, transfer_exp_list, save_fig_paths):
    if i < 6:
        i += 1
        continue



    if transfer_exp == "QAP":
        def get_type(instance_name):
            return instance_name.split("_")[0][0]

    elif transfer_exp == "PERMUPROB":
        def get_type(instance_name):
            return instance_name.split(".")[-1]

    elif transfer_exp == "QAP_MULTI":
        def get_type(instance_name):
            type_name = ""
            if "A" in instance_name:
                type_name += "A"
            if "B" in instance_name:
                type_name += "B"
            if "C" in instance_name:
                type_name += "C"
            if len(type_name)==2:
                type_name = type_name[0] + "_" + type_name[1]
            return type_name

    elif transfer_exp == "LOO16" or transfer_exp == "Transfer16OnlyOne":
        PROBLEM_TYPES = ["bowl", "valley", "plate","multiopt"]
        def get_type(instance_name):
            problem_index = int(instance_name.strip("_"))
            type_name = PROBLEM_TYPES[(problem_index - 1) // 4]
            return type_name


    print(input_txt.split("/")[-1])








    scores = []

    data_frame = pd.DataFrame(columns=["score", "train_name", "test_name", "train_type", "test_type"])

    with open(input_txt) as f:
        for line in f:

            # Trained only in one
            if transfer_exp == "Transfer16OnlyOne":
                if not "Only" in line:
                    continue
                line = line.split(",")
                line = [el.strip("[]") for el in line]
                train_name = "_"+str(line[2].split("TrainOnlyInF_")[1].split("_best.controller")[0])+"_"
                test_name = "_"+str(line[1])+"_"
                score = min(0,float(line[0]))


            # Continuous LOO16 
            elif transfer_exp == "LOO16":
                if "Only" in line:
                    continue
                line = line.split(",")
                line = [el.strip("[]") for el in line]
                train_name = "_"+str(line[2].split("LeaveOutF_")[1].split("_best.controller")[0])+"_"
                test_name = "_"+str(line[1])+"_"
                score = min(0,float(line[0]))
                # Its leave one out, so LeaveOutF_6_best.controller was actually trained in the rest of the problems of the same type (5 7 8).
                # This means that the controller can be tested in problems of different type or the problem left out. 
                if get_type(train_name) == get_type(test_name) and train_name != test_name:
                    continue



            # PERMUS (both qap and permuproblems)
            else:
                line = eval(line)
                train_name = line[2].split("/")[-1].split("_best.controlle")[-2]
                test_name = line[1].split("/")[-1]
                # Dont test the controller in an instance, when that instance was used to train it.
                if train_name in test_name:
                    continue
                score = line[0][0]


            train_type = get_type(train_name)
            test_type = get_type(test_name)

            # print("train_type",train_type, "test_type", test_type)


            new_row_df = pd.DataFrame([[score, train_name, test_name, train_type, test_type]], 
                            columns=["score", "train_name", "test_name", "train_type", "test_type"])

            data_frame = data_frame.append(new_row_df, ignore_index=True)

    def get_score(data_frame, train_name, test_name):
        return float(data_frame[(data_frame["train_name"] == train_name) & (data_frame["test_name"] == test_name)]["score"])



    def get_train_instances(data_frame):
        return sorted(list(data_frame["train_name"].unique()), key=lambda x: x.split(".")[-1] + "_" + x.split(".")[0])

    def get_test_instances(data_frame):
        return sorted(list(data_frame["test_name"].unique()), key=lambda x: x.split(".")[-1] + "_" + x.split(".")[0])


    def get_row_index(data_frame, train_name, test_name):
        element = data_frame[(data_frame["train_name"] == train_name) & (data_frame["test_name"] == test_name)]
        if len(element)==0:
            return None
        else:
            return element.index[0]



    # compute transferability
    transferability = []

    pos = 0
    neg = 0
    for idx in data_frame.index:
        row = data_frame.loc[idx,:]
        train_name = row["train_name"]
        test_name = row["test_name"]

        # # Cannot compute this because we are removed the scores where the same problem was used to train and test. 
        # norm_score = data_frame.iloc[get_row_index(data_frame, test_name, test_name)]["score"]

        sub_data_frame_with_certain_test_instance = data_frame[data_frame["test_name"] == test_name]
        
        average_score = mean(sub_data_frame_with_certain_test_instance["score"])
        stdev_score = stdev(sub_data_frame_with_certain_test_instance["score"])
        ranks = sub_data_frame_with_certain_test_instance["score"].rank(ascending=False)
        ranks = ranks - 1
        ranks = ranks / max(ranks)
        indx_in_selected = sub_data_frame_with_certain_test_instance["score"][sub_data_frame_with_certain_test_instance["score"]==data_frame.iloc[get_row_index(data_frame, train_name, test_name)]["score"]].index[0]

        
        # transferability.append(    (norm_score - row["score"]) / abs(norm_score)    ) # as defined on the paper
        # transferability.append(    (row["score"] - average_score) / stdev_score    )
        transferability.append(ranks[indx_in_selected])


    #     if norm_score > row["score"]:
    #             pos += 1
    #         else:
    #             neg += 1
    # print (pos / (pos + neg) * 100 , "%")


    data_frame.insert(1, "transferability", transferability, False) 
    print(data_frame[data_frame["test_name"]=="_1_"])




    # Average by instance type
    type_train = sorted(list(set(    [get_type(x) for x in data_frame["train_name"]]     )))
    type_test  = sorted(list(set(    [get_type(x) for x in data_frame["test_name"]]     )))


    transfer_result = pd.DataFrame(index=type_train, columns=type_test)



    for row in transfer_result.index:
        for col in transfer_result.columns:
            transfer_result.loc[(row,col)] = list()






    for train_name in get_train_instances(data_frame):
        for test_name in get_test_instances(data_frame):
            index = get_row_index(data_frame, train_name, test_name)
            if index is None:
                continue
            transf = data_frame.iloc[index]["transferability"]
            type_train = data_frame.iloc[index]["train_type"]
            type_test = data_frame.iloc[index]["test_type"]
            transfer_result.loc[(type_train,type_test)].append(transf)


    transfer_result = transfer_result.applymap(mean)

    pd.set_option('display.max_rows', 1000)
    print(transfer_result)
    # print(data_frame[data_frame["test_type"] == "tsp"])


    # Make the plots
    if transfer_exp == "LOO16" or transfer_exp == "Transfer16OnlyOne":

        matrix_data = np.zeros((16,16))
        for i in range(16):
            for j in range(16):
                train_name = "_" + str(i+1) + "_"
                test_name = "_" + str(j+1) + "_"
                value = data_frame[(data_frame["train_name"] == train_name) & (data_frame["test_name"] == test_name)]["transferability"]
                
                value = float(value) if len(value)>0 else np.NAN

                matrix_data[i,j] = value
        ax = sns.heatmap(matrix_data, linewidth=0.5, xticklabels=[str(i) for i in range(1,16+1)], yticklabels=[str(i) for i in range(1,16+1)])
        ax.xaxis.tick_top() # x axis on top
        ax.xaxis.set_label_position('top')
        ax.set_xlabel("Test instance")
        ax.set_ylabel("Training instance")
        plt.savefig("experimentResults/transfer_16_continuous_problems/results/figures/"+transfer_exp+"_heatmap.pdf")
        plt.close()




    # print(get_score(data_frame, "N-t65d11xx_150.lop", "pr136.tsp"))
    # print(get_type("pr136.tsp"))
    # print(get_train_instances(data_frame))
    # print(get_test_instances(data_frame))
    # print(get_row_index(data_frame, "N-t65d11xx_150.lop", "pr136.tsp"))
    # print(data_frame.iloc[478])




    data = pd.DataFrame(transfer_result)
    data.to_csv(save_fig_path + "_transferability_"+ transfer_exp + "_" + input_txt.split("/")[-1].split(".")[0] + ".csv", float_format="%.4f")