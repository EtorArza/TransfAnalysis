from statistics import mean, variance, stdev, median
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from seaborn import clustermap
import seaborn as sns
import random
import fnmatch
import matplotlib
import subprocess
from sklearn.datasets import make_biclusters
from sklearn.cluster import SpectralCoclustering
from sklearn.metrics import consensus_score
from scipy.cluster.hierarchy import dendrogram, linkage
import re
from tqdm import tqdm as tqdm


save_fig_path = "experimentResults/"



save_fig_paths = [
"experimentResults/transfer_permus_qap/results/figures/",
"experimentResults/transfer_permus_problems/results/figures/",
"experimentResults/transfer_16_continuous_problems/results/figures/",
"experimentResults/transfer_generated_continuous/results/figures/",
]


txt_paths = [
"experimentResults/transfer_permus_qap/results/response.txt",
"experimentResults/transfer_permus_problems/results/response.txt",
"experimentResults/transfer_16_continuous_problems/results/response.txt",
"experimentResults/transfer_generated_continuous/results/response.txt",
]


transfer_exp_list =[
"QAP",
"PERMUPROB",
"Transfer16OnlyOne",
"transferGenerated"
]

for input_txt, transfer_exp, save_fig_path in zip(txt_paths, transfer_exp_list, save_fig_paths):
    
    subprocess.run(f"mkdir -p {save_fig_path}", shell=True) # write out into log.txt

    def remove_seed_from_instance_name(instance_name):
        if 'seed' not in instance_name:
            print("ERROR: Controller name should include _seedk_best.controller, where k is the seed used to train the controller.")
            exit(1)
        instance_name = re.sub(r"seed\d*_", '', instance_name)
        return instance_name

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
        PROBLEM_TYPES = ["bowl", "multiopt"]
        def get_type(instance_name):
            problem_index = int(instance_name.split("seed")[0].strip("_"))
            type_name = PROBLEM_TYPES[(problem_index - 1) // 6]
            return type_name

    elif transfer_exp == "transferGenerated":
        def get_type(instance_name):
            return instance_name
            


    print(input_txt)








    responses = []

    data_frame = pd.DataFrame(columns=["response", "train_name", "test_name", "train_type", "test_type", "train_seed"])

    print(f"Reading file {input_txt}")
    with open(input_txt) as f:
        for line in f:
            train_seed = line.split("seed")[-1].split("_")[0]
            line = remove_seed_from_instance_name(line)
            # Trained only in one
            if transfer_exp == "Transfer16OnlyOne":
                if not "Only" in line:
                    continue
                line = eval(line)
                train_name = "_"+str(line[0].split("TrainOnlyInF_")[1].split("_best.controller")[0])+"_"
                test_name = "_"+str(line[1])+"_"
                response = np.array(line[-1])

            elif transfer_exp == "transferGenerated":
                line = eval(line)
                train_name = line[0].split("NLO_")[-1].split("_best.controlle")[0]
                test_name = str(line[-1])
                response = np.array(line[-2])


            # PERMUS (both qap and permuproblems)
            elif transfer_exp == "QAP_MULTI" or transfer_exp == "QAP" or transfer_exp=="PERMUPROB":
                line = eval(line)
                train_name = line[0].split("/")[-1].split("_best.controlle")[-2]
                test_name = line[1].split("/")[-1]
                # # Dont test the controller in an instance, when that instance was used to train it.
                # if train_name in test_name:
                #     continue
                response = np.array(line[2])

            else:
                print(f"ERROR: transfer_exp = {transfer_exp} not recognized!")
                exit(1)

            train_type = get_type(train_name)
            test_type = get_type(test_name)

            # print("train_type",train_type, "test_type", test_type)


            new_row_df = pd.DataFrame([[response, train_name, test_name, train_type, test_type, train_seed]], 
                            columns=["response", "train_name", "test_name", "train_type", "test_type", "train_seed"])
            data_frame = data_frame.append(new_row_df, ignore_index=True)

    def sort_key(x):
        if x.isnumeric():
            return float(x)
        elif x.strip("_ ").isnumeric():
            return float(x.strip("_ "))
        else:
            res = ""
            res += x.split(".")[-1]
            res += x
            return res


    def get_train_instances(data_frame):
        return sorted(list(data_frame["train_name"].unique()), key=sort_key)

    def get_test_instances(data_frame):
        return sorted(list(data_frame["test_name"].unique()), key=sort_key)


    def get_row_index(data_frame, train_name, test_name):
        element = data_frame[(data_frame["train_name"] == train_name) & (data_frame["test_name"] == test_name)]
        if len(element)==0:
            return None
        else:
            return element.index[0]


    def nicefyInstanceName(x: str):
        if transfer_exp == "Transfer16OnlyOne":
            return "$A_{"+x.strip("_") + "}$"
        elif transfer_exp == "QAP":
            return x.split("_")[0]
        else:
            return x.strip("_")

    # compute average distance between responses in train/test

    all_seeds = list(data_frame["train_seed"].unique()) 
    data_frame.insert(1, "transferability", [-1]*len(data_frame.index), False)

    distance_between_same_train_list_diff_seed = list()
    distance_between_same_train_list_same_seed = list()
    distance_between_both_diff_list = list()
    distance_between_same_test_list = list()

    def L1_distance_between_responses(resp1, resp2):
        return sum(abs(resp1 - resp2))


    train_names = sorted(data_frame["train_name"].unique(), key=sort_key)
    test_names = sorted(data_frame["test_name"].unique(), key=sort_key)

    N_distances_AVERAGED_per_instance = 500
    np.random.seed(3)
 
    # Compute avg response L1 distance between same train instances & different train seed.
    for train_name in tqdm(train_names):
        for reps in range(N_distances_AVERAGED_per_instance):
            seed1 = np.random.choice(all_seeds)
            seed2 = np.random.choice([el for el in all_seeds if el != seed1])
            r1_list = data_frame[(data_frame["train_name"] == train_name) & (data_frame["train_seed"] == seed1)]["response"]
            r2_list = data_frame[(data_frame["train_name"] == train_name) & (data_frame["train_seed"] == seed2)]["response"]
            r1 = np.random.choice(r1_list)
            r2 = np.random.choice(r2_list)
            distance_between_same_train_list_diff_seed.append(L1_distance_between_responses(r1,r2))
    distance_between_same_train_diff_seed = np.average(distance_between_same_train_list_diff_seed)


    # Compute avg response L1 distance between same test instances
    for test_name in tqdm(test_names):
        for reps in range(N_distances_AVERAGED_per_instance):
            seed1 = np.random.choice(all_seeds)
            seed2 = np.random.choice([el for el in all_seeds if el != seed1])
            r1_list = data_frame[(data_frame["test_name"] == test_name) & (data_frame["train_seed"] == seed1)]["response"]
            r2_list = data_frame[(data_frame["test_name"] == test_name) & (data_frame["train_seed"] == seed2)]["response"]
            r1 = np.random.choice(r1_list)
            r2 = np.random.choice(r2_list)
            distance_between_same_test_list.append(L1_distance_between_responses(r1,r2))

    distance_between_same_test = np.average(distance_between_same_test_list)

 
    # Compute avg response L1 distance between same train instances & same train seed.
    for train_name in tqdm(train_names):
        for reps in range(N_distances_AVERAGED_per_instance):
            seed1 = np.random.choice(all_seeds)
            seed2 = seed1
            r1_list = data_frame[(data_frame["train_name"] == train_name) & (data_frame["train_seed"] == seed1)]["response"]
            r2_list = data_frame[(data_frame["train_name"] == train_name) & (data_frame["train_seed"] == seed2)]["response"]

            r1 = np.random.choice(r1_list)
            r2 = np.random.choice(r2_list)

            k = 0
            while(sum(r1) == sum(r2)):
                r2 = np.random.choice(r2_list)
   

            distance_between_same_train_list_same_seed.append(L1_distance_between_responses(r1,r2))
    distance_between_same_train_same_seed = np.average(distance_between_same_train_list_same_seed)

    # Compute avg response L1 distance between diff train instances & diff train seed.
    for train_name in tqdm(train_names):
        for reps in range(N_distances_AVERAGED_per_instance):
            seed1 = np.random.choice(all_seeds)
            seed2 = seed1
            test_name = np.random.choice([el for el in test_names if el != train_name])
            
            r1_list = data_frame[(data_frame["train_name"] == train_name) & (data_frame["test_name"] == test_name) & (data_frame["train_seed"] == seed1)]["response"]
            r2_list = data_frame[(data_frame["train_name"] != train_name) & (data_frame["test_name"] != test_name) & (data_frame["train_seed"] == seed2)]["response"]

            r1 = np.random.choice(r1_list)
            r2 = np.random.choice(r2_list)

            k = 0
            while(sum(r1) == sum(r2)):
                r2 = np.random.choice(r2_list)
   

            distance_between_both_diff_list.append(L1_distance_between_responses(r1,r2))
    distance_between_both_diff_same_seed = np.average(distance_between_both_diff_list)


    print("---------")
    print("Problem name: ", transfer_exp)
    print("Avg L1 distance among responses TRAINED IN SAME instance (diff train seed):", distance_between_same_train_diff_seed)
    print("Avg L1 distance among responses TRAINED IN SAME instance (same train seed):", distance_between_same_train_same_seed)
    print("Avg L1 distance among responses TESTED IN SAME instance:", distance_between_same_test)
    print("Avg L1 distance among responses TRAINED & TESTED IN DIFFERENT instances:", distance_between_both_diff_same_seed)
    print("---------")

    continue




    for train_seed in all_seeds:

        sub_df = data_frame[data_frame["train_seed"] == train_seed]
        sub_df = sub_df.reset_index()

        for idx in sub_df.index:
            row = sub_df.loc[idx,:]
            train_name = row["train_name"]
            test_name = row["test_name"]

            # # Cannot compute this because we are removed the responses where the same problem was used to train and test. 
            # norm_response = sub_df.iloc[get_row_index(sub_df, test_name, test_name)]["response"]

            sub_sub_df_with_certain_test_instance = sub_df[sub_df["test_name"] == test_name]
            
            average_response = mean(sub_sub_df_with_certain_test_instance["response"])



    pd.set_option('display.max_rows', 1000)
    # print(data_frame[data_frame["test_type"] == "tsp"])

    # Make the plots
    if True:# transfer_exp == "LOO16" or transfer_exp == "Transfer16OnlyOne":

        train_names = sorted(data_frame["train_name"].unique(), key=sort_key)
        test_names = sorted(data_frame["test_name"].unique(), key=sort_key)

        print(train_names)

        nice_train_names = list(map(nicefyInstanceName, train_names))
        nice_test_names = list(map(nicefyInstanceName, test_names))



        m = len(train_names)
        n = len(test_names)

        matrix_data = np.zeros((m,n))
        for i in range(m):
            for j in range(n):
                train_name = train_names[i]
                test_name = test_names[j]
                value = data_frame[(data_frame["train_name"] == train_name) & (data_frame["test_name"] == test_name)]["transferability"]
                value = value.mean() if len(value) > 0 else np.NAN

                matrix_data[i,j] = value
        #sns.set_palette(sns.color_palette("viridis", as_cmap=True))

        sns.set(font_scale={"Transfer16OnlyOne": 1.4,
                            "transferGenerated": 1.4,
                            "QAP": 0.7,
                            "PERMUPROB": 0.7
                            }[transfer_exp]
                )
        ax = sns.heatmap(matrix_data, linewidth=0.5, xticklabels=nice_train_names, yticklabels=nice_test_names, vmin=0, vmax=1, cmap='viridis')


        ax.xaxis.tick_top() # x axis on top
        ax.xaxis.set_label_position('top')
        plt.xticks(rotation = 90)
        ax.set_xlabel("Test instance")
        ax.set_ylabel("Training instance")
        plt.tight_layout()
        plt.savefig(save_fig_path+transfer_exp+"_heatmap.pdf")
        plt.close()

        # https://seaborn.pydata.org/generated/seaborn.clustermap.html -> Plot a matrix dataset as a hierarchically-clustered heatmap.
        clust_df = pd.DataFrame(matrix_data, index=nice_train_names, columns=nice_test_names)
        clust_df = clust_df.rename_axis(index="Training instance", columns="Test instance")

        # with the single method, the distance from a point in space to the cluster is considered to be
        # the minimum distance to every point in the cluster. 
        # The cityblock metric is the L1 (taxicab or tophat) metric
        print(clust_df)
        z = linkage(clust_df, method="single", metric="cityblock", optimal_ordering=True)
        # ax = sns.clustermap(clust_df,metric='cityblock', method="single")
        sns.set(font_scale={"Transfer16OnlyOne": 2.0,
                            "transferGenerated": 2.0,
                            "QAP": 1.0,
                            "PERMUPROB": 1.0
                            }[transfer_exp]
                )        
        ax = sns.clustermap(clust_df, row_linkage=z, col_linkage=z, cmap='viridis')
        ax.ax_heatmap.xaxis.set_ticks_position('top')
        ax.ax_heatmap.xaxis.set_label_position('top')
        ax.ax_heatmap.yaxis.set_ticks_position('left')
        ax.ax_heatmap.yaxis.set_label_position('left')
        ax.ax_heatmap.tick_params(axis="x", labelrotation = 90)

        ax.cax.set_visible(True)
        ax.ax_row_dendrogram.set_visible(False)
        ax.ax_col_dendrogram.set_visible(False)
        ax.ax_row_dendrogram.set_xlim([0,0])
        plt.tight_layout()
        plt.savefig(save_fig_path+transfer_exp+"_clustered_"+"_heatmap.pdf")
        plt.close()


    # print(get_response(data_frame, "N-t65d11xx_150.lop", "pr136.tsp"))
    # print(get_type("pr136.tsp"))
    # print(get_train_instances(data_frame))
    # print(get_test_instances(data_frame))
    # print(get_row_index(data_frame, "N-t65d11xx_150.lop", "pr136.tsp"))
    # print(data_frame.iloc[478])

