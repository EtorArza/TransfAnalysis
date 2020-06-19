from statistics import mean, variance, stdev, median
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from seaborn import clustermap
from scipy.ndimage.filters import gaussian_filter
from copy import deepcopy
import random
import fnmatch
import matplotlib

# save in figures local folder
save_fig_path = "/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus/results/4by4_permu_problems/figures/"
# save_fig_path = "/home/paran/Dropbox/BCAM/02_NEAT_permus/paper/images/permu_problems_transfer/"


txt_paths = [
"/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus_multi/results/qap_cut_multi_vs_mono/score_multi_instance_cut_qap.txt",
"/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus/results/4by4_permu_problems/result_score_transfer_permuproblem_0_1s_2h.txt",
"/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus/results/4by4_permu_problems/result_score_transfer_permuproblem_0_25s_1h.txt",
"/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus/results/4by4_permu_problems/result_score_transfer_permuproblem_0_1s_12h.txt",
"/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus/results/transfer_qap_with_cut_instances/result_score_transfer_qap_0_1s_2h.txt",
"/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus/results/transfer_qap_with_cut_instances/result_score_transfer_qap_0_25s_1h.txt",
"/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus/results/transfer_qap_with_cut_instances/result_score_transfer_qap_0_1s_12h.txt"
]


transfer_exp_list =[
"QAP_MULTI",
"PERMUPROB",
"PERMUPROB",
"PERMUPROB",
"QAP",
"QAP",
"QAP"
]

for input_txt, transfer_exp in zip(txt_paths, transfer_exp_list):
    



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

    print(input_txt.split("/")[-1])








    scores = []

    data_frame = pd.DataFrame(columns=["score", "train_name", "test_name", "train_type", "test_type"])

    with open(input_txt) as f:
        for line in f:
            line = eval(line)
            score = line[0][0]

            train_name = line[2].split("/")[-1].split("_best.controlle")[-2]
            test_name = line[1].split("/")[-1]

            train_type = get_type(train_name)
            test_type = get_type(test_name)



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
        return data_frame[(data_frame["train_name"] == train_name) & (data_frame["test_name"] == test_name)].index[0]



    # compute transferability
    transferability = []

    pos = 0
    neg = 0
    for idx in data_frame.index:
        row = data_frame.loc[idx,:]
        train_name = row["train_name"]
        test_name = row["test_name"]

        norm_score = data_frame.iloc[get_row_index(data_frame, test_name, test_name)]["score"]
        average_score = mean(data_frame[data_frame["test_name"] == test_name]["score"])
        stdev_score = stdev(data_frame[data_frame["test_name"] == test_name]["score"])

        if norm_score > row["score"]:
            pos += 1
        else:
            neg += 1
        transferability.append(    (norm_score - row["score"]) / abs(norm_score)    ) # as defined on the paper
        # transferability.append(    (row["score"] - average_score) / stdev_score    ) 



    print (pos / (pos + neg) * 100 , "%")


    data_frame.insert(1, "transferability", transferability, False) 


    # discard if train is contained in test
    rows_to_keep = [False if data_frame.iloc[i]["test_name"] in data_frame.iloc[i]["train_name"] else True for i in range(data_frame.shape[0])]
    data_frame = data_frame[rows_to_keep]
    data_frame.reset_index(inplace=True, drop=True) 


    # Average by instance type
    type_train = sorted(list(set(    [get_type(x) for x in data_frame["train_name"]]     )))
    type_test  = sorted(list(set(    [get_type(x) for x in data_frame["test_name"]]     )))


    transfer_result = pd.DataFrame(index=type_train, columns=type_test)



    for row in transfer_result.index:
        for col in transfer_result.columns:
            transfer_result.loc[(row,col)] = list()






    for train_name in get_train_instances(data_frame):
        for test_name in get_test_instances(data_frame):
            if test_name in train_name:
                continue
            index = get_row_index(data_frame, train_name, test_name)
            transf = data_frame.iloc[index]["transferability"]
            type_train = data_frame.iloc[index]["train_type"]
            type_test = data_frame.iloc[index]["test_type"]
            transfer_result.loc[(type_train,type_test)].append(transf)


    transfer_result = transfer_result.applymap(mean)

    pd.set_option('display.max_rows', 1000)
    # print(transfer_result)
    # print(data_frame[data_frame["test_type"] == "tsp"])


    # print(get_score(data_frame, "N-t65d11xx_150.lop", "pr136.tsp"))
    # print(get_type("pr136.tsp"))
    # print(get_train_instances(data_frame))
    # print(get_test_instances(data_frame))
    # print(get_row_index(data_frame, "N-t65d11xx_150.lop", "pr136.tsp"))
    # print(data_frame.iloc[478])




    data = pd.DataFrame(transfer_result)
    data.to_csv(save_fig_path + "_transferability_"+ transfer_exp + "_" + input_txt.split("/")[-1].split(".")[0] + ".csv", float_format="%.4f")