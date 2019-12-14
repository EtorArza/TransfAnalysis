import pandas as pd
import numpy as np
from sklearn.cluster import KMeans
from scipy.cluster import hierarchy
from scipy.cluster.hierarchy import dendrogram, linkage
from scipy.cluster.hierarchy import fcluster
import numpy as np

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
            cont_size = "size_30" if "cut30" in controller else "size_60"
            inst_size = "size_30" if "cut30" in instance else "size_60"
            cont_class = "sko" if "sko" in controller else "tai_a" if "0a" in controller else "tai_b"
            inst_class = "sko" if "sko" in instance else "tai_a" if "0a" in instance else "tai_b"
            indexes.append(cont_size + "_" + cont_class + " - " + inst_size + "_" + inst_class+"_"+str(j))
        elif i == 1:
            i = 0
            list_of_values = eval(line.split("|")[-1].strip("\n"))
            data.append([cont_size, cont_class, inst_size, inst_class] + list_of_values)


df = pd.DataFrame(data=data, columns=["cont_size", "cont_class", "inst_size", "inst_class"] + ["x_" + str(i) for i in range(len(list_of_values))], index=indexes)


# K MEANS, 2 clusters

model = KMeans(n_clusters=2, random_state=0).fit(df.iloc[:,4:])


df = df.assign(cluster=pd.Series(model.labels_).values)


size_classes = ["size_30", "size_60"]
type_classes = ["tai_a", "sko", "tai_b"]


class_balance_instances = pd.DataFrame(data=np.zeros((len(size_classes), len(type_classes))), index = size_classes, columns = type_classes)
class_balance_controllers = pd.DataFrame(data=np.zeros((len(size_classes), len(type_classes))), index = size_classes, columns = type_classes)



# class_balance_instances.loc["size_30", "tai_a"] = 0.0


N_OF_MAX_IN_ONE_position = 2 * 3 * ( 3 * 3) # number of possible combination of controller/instance + number of 

for indx, line in df.iterrows():
    class_balance_instances.loc[line["inst_size"], line["inst_class"]] += line["cluster"] / N_OF_MAX_IN_ONE_position
    class_balance_controllers.loc[line["cont_size"], line["cont_class"]] += line["cluster"] / N_OF_MAX_IN_ONE_position



print("class_balance_instances:\n", class_balance_instances, "\n\n----------------\n\n")
print("class_balance_controllers:\n", class_balance_controllers, "\n\n-----------------\n\n")



import matplotlib
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42
from matplotlib import pyplot as plt


save_fig_path = "/home/paran/Dropbox/BCAM/02_NEAT_permus/paper/images/analyze_outputs/"



def save_fig(d, fig_title, fig_path):

    data = d.copy(deep=True)

    plt.pcolor(data, vmin=0, vmax=1)
    plt.yticks(np.arange(0.5, len(data.index), 1), data.index)
    plt.xticks(np.arange(0.5, len(data.columns), 1), data.columns, rotation = 90)
    plt.ylabel("instance size")
    plt.xlabel("instance type")
    plt.colorbar()
    plt.title(fig_title)
    plt.tight_layout()
    plt.savefig(fig_path)
    plt.close()



save_fig(class_balance_instances, "Percentage of cases on cluster 1 \n classified by the test instance", save_fig_path+"by_test.pdf")
save_fig(class_balance_controllers, "Percentage of cases on cluster 1 \n classified by the train instance", save_fig_path+"by_train.pdf")





# K MEANS, 6 clusters

model_6 = KMeans(n_clusters=6, random_state=0).fit(df.iloc[:,4:-1])
df = df.assign(cluster_6_classes=pd.Series(model_6.labels_).values)



print(df)

print(df[df["cluster_6_classes"] == 1])






exit(1)












# hierarchical clustering, plot dendogram with colors
Z = linkage(df.iloc[:,4:], 'ward')
res = dendrogram(Z, labels=df.index, leaf_rotation=0, orientation="left", color_threshold=7.5, above_threshold_color='grey', no_labels = True)
plt.title("dendogram of the hierarchical clustering")
plt.savefig(save_fig_path+"hierarchical_clustering_average_response")









classes = []

for indx, line in df.iterrows():

    pos = res['ivl'].index(indx)
    classes.append(res["color_list"][pos-1])



df = df.assign(colors=pd.Series(classes).values)


with pd.option_context('display.max_rows', None, 'display.max_columns', None):  # more options can be specified also
    print(df)

classes = list(df["assignments"])

count = [classes.count(i) for i in range(1, 7)]

print(count)

class_color_mapping = dict()


