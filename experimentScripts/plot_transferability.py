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

print("__name__ =", __name__)

np.random.seed(2)

def get_random_value(problem_index, dim=2):
    rnd_x_str = ",".join([str(el) for el in np.random.random(dim)])
    exec_res=subprocess.run(f"./neat -evaluate-continuous-problem {problem_index} {rnd_x_str}",shell=True, capture_output=True)  
    return float(str(exec_res.stdout).strip("'b"))

def rand_search(problem_index, n_sols):
    return max((get_random_value(problem_index) for _ in range(n_sols)))


n = 12
nseeds = 10
n_sols = 20
matrix_data = np.zeros((n, n))
matrix_data_mean = np.zeros((n, n))
save_fig_path = "experimentResults/"


def get_values_to_choose_from(j, n):
    values_to_choose_from = np.array([rand_search(j, n_sols) for _ in range(n)])
    values_to_choose_from = np.argsort(-values_to_choose_from) / (n-1)
    return values_to_choose_from

if __name__ == "__main__":
    print("Running main random search.")

    for j in tqdm(range(n)): # for each test
        
        values_to_choose_from = get_values_to_choose_from(j, n)
        values_to_choose_from_avg = np.zeros_like(values_to_choose_from)
        for _ in range(nseeds):
            values_to_choose_from_avg += get_values_to_choose_from(j, n)
        values_to_choose_from_avg /= nseeds

        for i in range(n): # for each train
            train_name = i
            test_name = j
            only_one_value = values_to_choose_from[i]
            mean_value = values_to_choose_from_avg[i]
            matrix_data[i,j] = only_one_value
            matrix_data_mean[i,j] = mean_value

    for m, name in zip([matrix_data, matrix_data_mean] , ["randomsearch", "randomsearch_median_10seeds"]):

        sns.set(font_scale = 1.4)
        #sns.set_palette(sns.color_palette("viridis", as_cmap=True))
        tick_labels = ["$A_{" + str(el) + "}$" for el in range(1,n+1)]
        ax = sns.heatmap(m, linewidth=0.5, xticklabels=tick_labels, yticklabels=tick_labels, vmin=0, vmax=1, cmap='viridis')

        ax.xaxis.tick_top() # x axis on top
        ax.xaxis.set_label_position('top')

        plt.xticks(rotation = 90)
        ax.set_xlabel("Test instance")
        ax.set_ylabel("Training instance")
        plt.tight_layout()
        plt.savefig(save_fig_path + name + "_heatmap.pdf")
        plt.close()



# biclustering

def measure_matrix_loss(matrix):
    n = matrix.shape[0]
    loss = 0
    for i in range(1,n):
        loss += abs(matrix[i,j]- matrix[i-1,j])
        loss += abs(matrix[i,j]- matrix[i,j-1])
        loss += abs(matrix[i,j]- matrix[i-1,j-1])
    return loss


def change_distance_matrix_indices(matrix, permu):
    n = matrix.shape[0]
    t = np.eye(n)[permu]
    return t * permu * t



# save_fig_path = "/home/paran/Dropbox/BCAM/02_NEAT_permus/paper/images/permu_problems_transfer/"

save_fig_paths = [
"experimentResults/transfer_permus_qap/results/figures/",
"experimentResults/transfer_permus_problems/results/figures/",
"experimentResults/transfer_16_continuous_problems/results/figures/",
"experimentResults/transfer_generated_continuous/results/figures/",
]


txt_paths = [
"experimentResults/transfer_permus_qap/results/score.txt",
"experimentResults/transfer_permus_problems/results/score.txt",
"experimentResults/transfer_16_continuous_problems/results/score.txt",
"experimentResults/transfer_generated_continuous/results/score.txt",
]


transfer_exp_list =[
"QAP",
"PERMUPROB",
"Transfer16OnlyOne",
"transferGenerated"
]
every_data_frame_list = []
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








    scores = []

    data_frame = pd.DataFrame(columns=["score", "train_name", "test_name", "train_type", "test_type", "train_seed"])

    print(f"Reading file {input_txt}")
    with open(input_txt) as f:
        for line in f:
            train_seed = line.split("seed")[-1].split("_")[0]
            line = remove_seed_from_instance_name(line)
            # Trained only in one
            if transfer_exp == "Transfer16OnlyOne":
                if not "Only" in line:
                    continue
                line = line.split(",")
                line = [el.strip("[]") for el in line]
                train_name = "_"+str(line[2].split("TrainOnlyInF_")[1].split("_best.controller")[0])+"_"
                test_name = "_"+str(line[1])+"_"
                score = float(line[0])


            # Continuous LOO16 
            elif transfer_exp == "LOO16":
                if "Only" in line:
                    continue
                line = line.split(",")
                line = [el.strip("[]") for el in line]
                train_name = "_"+str(line[2].split("LeaveOutF_")[1].split("_best.controller")[0])+"_"
                test_name = "_"+str(line[1])+"_"
                score = float(line[0])
                # Its leave one out, so LeaveOutF_6_best.controller was actually trained in the rest of the problems of the same type (5 7 8).
                # This means that the controller can be tested in problems of different type or the problem left out. 
                if get_type(train_name) == get_type(test_name) and train_name != test_name:
                    continue

            elif transfer_exp == "transferGenerated":
                line = line.split(",")
                line = [el.strip("[]\n") for el in line]
                train_name = line[2].split("NLO_")[-1].split("_best.controlle")[0]
                test_name = line[-1]
                score = float(line[0])


            # PERMUS (both qap and permuproblems)
            elif transfer_exp == "QAP_MULTI" or transfer_exp == "QAP" or transfer_exp=="PERMUPROB":
                line = eval(line)
                train_name = line[2].split("/")[-1].split("_best.controlle")[-2]
                test_name = line[1].split("/")[-1]
                # # Dont test the controller in an instance, when that instance was used to train it.
                # if train_name in test_name:
                #     continue
                score = line[0][0]

            else:
                print(f"ERROR: transfer_exp = {transfer_exp} not recognized!")
                exit(1)

            train_type = get_type(train_name)
            test_type = get_type(test_name)

            # print("train_type",train_type, "test_type", test_type)


            new_row_df = pd.DataFrame([[score, train_name, test_name, train_type, test_type, train_seed]], 
                            columns=["score", "train_name", "test_name", "train_type", "test_type", "train_seed"])
            data_frame = data_frame.append(new_row_df, ignore_index=True)

    every_data_frame_list.append(data_frame)

    if __name__ == "__main__":
        print("Running main.")

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
                name = x.split("_")[0]
                new_name = name[0] + str(["A","B","C"].index(name[0]) * 7 + int(name[1]))
                return new_name
            else:
                return x.strip("_")

        if transfer_exp == "Transfer16OnlyOne":
            scores_dict = dict()
            for index, row in data_frame.iterrows():
                train_name = row["train_name"].split("seed")[0]
                test_name = row["test_name"]
                traintest_touple = (train_name, test_name)
                score = row["score"]
                scores_dict[traintest_touple] = scores_dict[traintest_touple] + [score] if traintest_touple in scores_dict else [score]    

            BOXPLOT_data_frame = pd.DataFrame(columns=["score", "train_name", "test_name", "train_type", "test_type"])




            # plot all boxplots
            sns.set(font_scale = 1)
            fig, ax_list = plt.subplots(6,2)

            for test_index in range(1,13):
                ax = ax_list.flatten()[test_index-1]
                ax.boxplot([scores_dict[(f"_{i}_", f"_{test_index}_")] for i in range(1, 13)], labels=[ r"$"+r"A_{"+ str(el) + r"}$" for el in range(1,13)])
                #ax.set_xlabel("Train problem") 
                ax.set_title("Tested in problem $A_{" + f"{test_index}" + "}$") 

            fig.set_size_inches(8, 11)
            fig.tight_layout()
            fig.savefig(save_fig_path+"boxplot_for_each_test_instance.pdf")
            plt.close()
    
            for (train_name, test_name), score in scores_dict.items():
                
                train_type = get_type(train_name)
                test_type = get_type(test_name)
                score = median(score)
                new_row_df = pd.DataFrame([[score, train_name, test_name, train_type, test_type]], 
                                    columns=["score", "train_name", "test_name", "train_type", "test_type"])

                BOXPLOT_data_frame = data_frame.append(new_row_df, ignore_index=True)
        sns.set(font_scale = 1.4)

        # compute transferability

        if transfer_exp == "Transfer16OnlyOne":
            print(data_frame[data_frame["train_seed"] == "2"])

        all_seeds = list(data_frame["train_seed"].unique()) 
        data_frame.insert(1, "transferability", [-1]*len(data_frame.index), False)

        for train_seed in all_seeds:
            transferability = []

            sub_df = data_frame[data_frame["train_seed"] == train_seed]
            sub_df = sub_df.reset_index()

            for idx in sub_df.index:
                row = sub_df.loc[idx,:]
                train_name = row["train_name"]
                test_name = row["test_name"]

                # # Cannot compute this because we are removed the scores where the same problem was used to train and test. 
                # norm_score = sub_df.iloc[get_row_index(sub_df, test_name, test_name)]["score"]

                sub_sub_df_with_certain_test_instance = sub_df[sub_df["test_name"] == test_name]
                
                average_score = mean(sub_sub_df_with_certain_test_instance["score"])
                stdev_score = stdev(sub_sub_df_with_certain_test_instance["score"])
                ranks = sub_sub_df_with_certain_test_instance["score"].rank(ascending=False)
                ranks = ranks - 1
                ranks = ranks / max(ranks)
                indx_in_selected = sub_sub_df_with_certain_test_instance["score"][sub_sub_df_with_certain_test_instance["score"]==sub_df.iloc[get_row_index(sub_df, train_name, test_name)]["score"]].index[0]

                # transferability.append(    (norm_score - row["score"]) / abs(norm_score)    ) # as defined on the paper
                # transferability.append(    (row["score"] - average_score) / stdev_score    )
                transferability.append(ranks[indx_in_selected])


            # print(data_frame.loc[data_frame['train_seed'] == train_seed, "transferability"])
            # print(np.array(transferability), len(np.array(transferability)))

            # Insert transferability column
            data_frame.loc[data_frame['train_seed'] == train_seed, "transferability"] = np.array(transferability)



            # # Average by instance type
            # type_train = sorted(list(set(    [get_type(x) for x in sub_df["train_name"]]     )))
            # type_test  = sorted(list(set(    [get_type(x) for x in sub_df["test_name"]]     )))


            # for train_name in get_train_instances(sub_df):
            #     for test_name in get_test_instances(sub_df):
            #         index = get_row_index(sub_df, train_name, test_name)
            #         if index is None:
            #             continue
            #         transf = sub_df.iloc[index]["transferability"]
            #         type_train = sub_df.iloc[index]["train_type"]
            #         type_test = sub_df.iloc[index]["test_type"]



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


        # print(get_score(data_frame, "N-t65d11xx_150.lop", "pr136.tsp"))
        # print(get_type("pr136.tsp"))
        # print(get_train_instances(data_frame))
        # print(get_test_instances(data_frame))
        # print(get_row_index(data_frame, "N-t65d11xx_150.lop", "pr136.tsp"))
        # print(data_frame.iloc[478])

