import pandas as pd
import numpy as np
from sklearn.cluster import KMeans
from scipy.cluster import hierarchy
from scipy.cluster.hierarchy import dendrogram, linkage
from scipy.cluster.hierarchy import fcluster
import numpy as np
import matplotlib
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42
from matplotlib import pyplot as plt


save_fig_path = "/home/paran/Dropbox/BCAM/02_NEAT_permus/paper/images/analyze_outputs/"
#save_fig_path = ""


def save_fig(d, fig_title, fig_path, scale_between_1_and_0 = False):

    data = d.copy(deep=True)
    print(data)

    if scale_between_1_and_0:
        plt.pcolor(data, vmin=0, vmax=1)
    else:
        plt.pcolor(data)
    plt.yticks(np.arange(0.5, len(data.index), 1), data.index)
    plt.xticks(np.arange(0.5, len(data.columns), 1), data.columns, rotation = 90)
    plt.ylabel("trained on")
    plt.xlabel("tested on")
    plt.colorbar()
    plt.title(fig_title)
    plt.tight_layout()
    plt.savefig(fig_path)
    plt.close()













data = []
indexes = []


i = 0
j = 100
with open("output_values.txt", "r") as f:
    for line in f:
        if i == 0:
            i = 1
            j+=1
            controller, instance = line.strip("\n").split("|")
            cont_size = "size=30" if "cut30" in controller else "size=60"
            inst_size = "size=30" if "cut30" in instance else "size=60"
            cont_class = "Sko" if "sko" in controller else "Taixxa" if "0a" in controller else "Taixxb"
            inst_class = "Sko" if "sko" in instance else "Taixxa" if "0a" in instance else "Taixxb"
            indexes.append(cont_size + "_" + cont_class + " - " + inst_size + "_" + inst_class+"_"+str(j))
        elif i == 1:
            i = 0
            list_of_values = eval(line.split("|")[-1].strip("\n"))
            data.append([cont_size, cont_class, inst_size, inst_class] + list_of_values)


df = pd.DataFrame(data=data, columns=["cont_size", "cont_class", "inst_size", "inst_class"] + ["x_" + str(i) for i in range(len(list_of_values))], index=indexes)



 





size_classes = ["size=30", "size=60"]
type_classes = ["Taixxa", "Sko", "Taixxb"]


for size in size_classes:
    sep_df = df[(df.ix[:,"cont_size"] == size) & (df.ix[:,"inst_size"] == size)]
    avg_distance_matrix_by_type_and_size_set = pd.DataFrame(data=np.zeros((len(type_classes) + 1, len(type_classes) + 1)), index = type_classes+["all"], columns = type_classes+["all"])

    for train_type in type_classes:
        for test_type in type_classes:
            behaviours = sep_df[(sep_df.ix[:,"cont_class"] == train_type) & (sep_df.ix[:,"inst_class"] == test_type)].iloc[:,4:]
            avge_L1_distance_to_mean_behaviour = abs(behaviours - behaviours.mean()).mean().mean()
            avg_distance_matrix_by_type_and_size_set.loc[train_type, test_type] = avge_L1_distance_to_mean_behaviour

    print("------------------------------------------------------------")
    for type_class in type_classes:

            # fix train instance type
            behaviours = sep_df[(sep_df.ix[:,"cont_class"] == type_class)].iloc[:,4:]
            avge_L1_distance_to_mean_behaviour = abs(behaviours - behaviours.mean()).mean().mean()
            avg_distance_matrix_by_type_and_size_set.loc[type_class, "all"] = avge_L1_distance_to_mean_behaviour
            print("fix train to",type_class, "->" , avge_L1_distance_to_mean_behaviour)

            # fix test instance type
            behaviours = sep_df[(sep_df.ix[:,"inst_class"] == type_class)].iloc[:,4:]
            avge_L1_distance_to_mean_behaviour = abs(behaviours - behaviours.mean()).mean().mean()
            avg_distance_matrix_by_type_and_size_set.loc["all", type_class] = avge_L1_distance_to_mean_behaviour

    avg_distance_matrix_by_type_and_size_set.loc["all","all"] = np.nan
    avg_distance_matrix_by_type_and_size_set = avg_distance_matrix_by_type_and_size_set.iloc[::-1]

    save_fig(avg_distance_matrix_by_type_and_size_set, "Average L1 distance to the mean behaviour", save_fig_path+"avg_L1_dist_to_mean_behaviour_with_size_fixed_at"+ str(size) +".pdf")




for each_type in type_classes:
    sep_df = df[(df.ix[:,"cont_class"] == each_type) & (df.ix[:,"inst_class"] == each_type)]
    avg_distance_matrix_by_type_and_size_set = pd.DataFrame(data=np.zeros((len(size_classes) + 1, len(size_classes) + 1)), index = size_classes+["all"], columns = size_classes+["all"])

    for train_size in size_classes:
        for test_size in size_classes:
            behaviours = sep_df[(sep_df.ix[:,"cont_size"] == train_size) & (sep_df.ix[:,"inst_size"] == test_size)].iloc[:,4:]
            avge_L1_distance_to_mean_behaviour = abs(behaviours - behaviours.mean()).mean().mean()
            avg_distance_matrix_by_type_and_size_set.loc[train_size, test_size] = avge_L1_distance_to_mean_behaviour
    for size_class in size_classes:

            # fix train instance type
            behaviours = sep_df[(sep_df.ix[:,"cont_size"] == size_class)].iloc[:,4:]
            avge_L1_distance_to_mean_behaviour = abs(behaviours - behaviours.mean()).mean().mean()
            avg_distance_matrix_by_type_and_size_set.loc[size_class, "all"] = avge_L1_distance_to_mean_behaviour

            # fix test instance type
            behaviours = sep_df[(sep_df.ix[:,"inst_size"] == size_class)].iloc[:,4:]
            avge_L1_distance_to_mean_behaviour = abs(behaviours - behaviours.mean()).mean().mean()
            avg_distance_matrix_by_type_and_size_set.loc["all", size_class] = avge_L1_distance_to_mean_behaviour

    avg_distance_matrix_by_type_and_size_set.loc["all","all"] = np.nan
    avg_distance_matrix_by_type_and_size_set = avg_distance_matrix_by_type_and_size_set.iloc[::-1]

    save_fig(avg_distance_matrix_by_type_and_size_set, "Average L1 distance to the mean behaviour", save_fig_path+"avg_L1_dist_to_mean_behaviour_with_type_fixed_at_"+ str(each_type) +".pdf")
