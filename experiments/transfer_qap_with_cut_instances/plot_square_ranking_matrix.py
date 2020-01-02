from statistics import mean, variance, stdev, median
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from seaborn import clustermap
from scipy.ndimage.filters import gaussian_filter
from copy import deepcopy
import random
import fnmatch


# save in figures local folder
#save_fig_path = "figures/"

# save in GECCO article dir
import matplotlib
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42
save_fig_path = "/home/paran/Dropbox/BCAM/02_NEAT_permus/paper/images/qap_transfer_cut/"




scores = []

with open("result_controllers.txt") as f:
    for line in f:
        values = [float(el) for el in line.split("]")[0].strip("[").split(",")]
        n = len(values)
        instance = line.split(",")[n].split(".")[0].split("/")[-1]
        controller = line.split(",")[n+1].split("/")[-1].split("_gen_")[0]
        scores.append([values, instance, controller])



inst_contr_dict = dict()
test_instances = []
for el in scores:
    if el[1] not in test_instances:
        test_instances.append(el[1])
    #inst_contr_dict[(el[1],el[2])] = (mean(el[0]) - RS_res) / (BK - RS_res)
    inst_contr_dict[(el[1],el[2])] = mean(el[0])




def xor(a, b):
    return (a and not b) or (not a and b)



def order(x):
    if "0a" in x:
        return x.split("_")[0] + "_A_" + x.split("_")[1]
    elif "sko" in x:
        return x.split("_")[0] + "_SKO_" + x.split("_")[1]
    elif "0b" in x:
        return x.split("_")[0] + "_B_" + x.split("_")[1]
    else:
        print("Error, x="+str(x)+" could not be found have sko 0a or 0b.")
        exit(1)


test_instances = sorted(test_instances, key=order)


train_instances = test_instances[:]



zero_data = np.zeros(shape=(len(train_instances),len(test_instances)))
d = pd.DataFrame(zero_data, columns=test_instances, index=train_instances)



for inst in test_instances:
    for contr in train_instances:
        d.ix[contr, inst] = inst_contr_dict[(inst, contr)]



def inverse(iterable):
    res = np.zeros(len(iterable))
    for i in range(len(iterable)):
        res[iterable[i]] = i
    return res


for column in d:
    d[column] = inverse(np.argsort(d[column]))
    d[column] -= mean(d[column])
    d[column] /= stdev(d[column]) # max(d[column])



# d = gaussian_filter(d, sigma=0.7)
# d = pd.DataFrame(d, columns=test_instances, index=train_instances)


n = d.shape[0]
m = d.shape[1]



# plt.show()

def get_loss(df):
    return sum([sum(abs(df.iloc[:,i] - df.iloc[:,i+1])) for i in range(m-1)])

def swap_three_cols(df):
    df = df.copy(deep=True)
    i,j,k = np.random.randint(m,size = 3)
    while i == j or j == k or k == i:
        i,j,k = np.random.randint(m,size = 3)
    #print(df)

    new_order = list(range(m))
    new_order[i], new_order[j], new_order[k] = j,i,k
    return df[df.columns[new_order]]

def ls(df):
    best_loss = 100000000000
    for _ in range(3000):
        df_copy = swap_three_cols(df)
        if get_loss(df_copy) < best_loss:
            df = df_copy
            best_loss = get_loss(df)
        print(get_loss(df))
    return df

"""
cur_loss = 10000000000000
best_loss = 100000000000
df_copy = d.copy(deep = True)
for i in range(15):
    print("..i..")
    df_copy = ls(df_copy)
    cur_loss = get_loss(df_copy)
    if cur_loss < best_loss:
        best_loss = cur_loss
        d = df_copy.copy(deep=True)
    new_order = list(range(m))
    random.shuffle(new_order)
    df_copy[df_copy.columns[new_order]]
"""


def rename_name(case_name, size_relevant=True, class_relevant=True):
    new_name = ""

    if class_relevant:
        if fnmatch.fnmatch(case_name, "*tai*a"):
            new_name += "Taixxa"
        elif fnmatch.fnmatch(case_name, "*sko*"):
            new_name += "Sko"
        elif fnmatch.fnmatch(case_name, "*tai*b"):
            new_name += "Taixxb"
        else:
            raise Warning("tai*a, sko or tai*b not found in case name "+str(case_name))

    if size_relevant:
        if "cut30" in case_name:
            new_name += "size=30"
        elif "cut60" in case_name:
            new_name += "size=60"
        else:
            raise Warning("cut30 or cut60 not found in case name "+str(case_name))

    return new_name


def save_fig(d, fig_title, fig_path,size_relevant=True, class_relevant=True):

    data = d.copy(deep=True)

    plt.pcolor(data, )
    yticks = [rename_name(el,size_relevant, class_relevant) for el in data.index]
    xticks = [rename_name(el,size_relevant, class_relevant) for el in data.columns]




    plt.yticks(np.arange(0.5, len(data.index), 1), yticks)
    plt.xticks(np.arange(0.5, len(data.columns), 1), xticks, rotation = 90)
    plt.ylabel("trained on")
    plt.xlabel("tested on")
    #plt.title("(average - RS) / (BK - RS)")
    #plt.title("normalized, gaussian smoothing, sigma = 0.7")
    #plt.title("rankings on test instances")
    plt.title(fig_title)
    plt.tight_layout()
    plt.savefig(fig_path)
    plt.close()





all_instances = [
"cut30_tai100a",
"cut30_tai60a",
"cut30_tai80a",
"cut30_tai100b",
"cut30_tai60b",
"cut30_tai80b",
"cut30_sko72",
"cut30_sko81",
"cut30_sko90",
"cut60_tai100a",
"cut60_tai60a",
"cut60_tai80a",
"cut60_tai100b",
"cut60_tai60b",
"cut60_tai80b",
"cut60_sko72",
"cut60_sko81",
"cut60_sko90"
]


small_instances = [ins for ins in all_instances if "cut30" in ins]
big_instances = [ins for ins in all_instances if "cut60" in ins]


taia_instances = [ins for ins in all_instances if "0a" in ins]
taib_instances = [ins for ins in all_instances if "0b" in ins]
sko_instances = [ins for ins in all_instances if "sko" in ins]





save_fig(d, "All normalized scores", save_fig_path+"all_norm.pdf", True, True) #all
save_fig(d.loc[small_instances,small_instances], "Small instances", save_fig_path+"small.pdf", False, True) # small instances
save_fig(d.loc[big_instances,big_instances], "Large instances", save_fig_path+"large.pdf", False, True) # small instances
save_fig(d.loc[taia_instances,taia_instances], "taia_instances", save_fig_path+"taia_instances.pdf", True, False) # taia instances
save_fig(d.loc[taib_instances,taib_instances], "taib_instances", save_fig_path+"taib_instances.pdf", True, False) # taib instances
save_fig(d.loc[sko_instances,sko_instances], "sko_instances", save_fig_path+"sko_instances.pdf", True, False) # sko instances
