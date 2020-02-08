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
#save_fig_path = "figures/"
save_fig_path = "/home/paran/Dropbox/BCAM/02_NEAT_permus/paper/images/permu_problems_transfer/"


#input_txt = "result_controllers_GECCO2020_version.txt"
input_txt = "result_controllers_journal_version.txt"






scores = []


with open(input_txt) as f:
    for line in f:
        values = [float(el) for el in line.split("]")[0].strip("[").split(",")]
        print(values)
        n = len(values)
        instance = line.split(",")[n].split(".")[0].split("/")[-1]
        controller = line.split(",")[n+1].split("/")[-1].split("_gen_")[0].split("with_")[1]
        scores.append([values, instance, controller])


inst_contr_dict = dict()
test_instances = []
for el in scores:
    if el[1] not in test_instances:
        test_instances.append(el[1])
    #inst_contr_dict[(el[1],el[2])] = (mean(el[0]) - RS_res) / (BK - RS_res)
    print((el[1],el[2]))
    inst_contr_dict[(el[1],el[2])] = mean(el[0])




def xor(a, b):
    return (a and not b) or (not a and b)



def rename_name(case_name, size_relevant=True, class_relevant=True):
    
    if "sko" in case_name:
        return "qap"
    
    if "tai" in case_name:
        if "_" in case_name:
            return "pfsp"
        else:
            return "qap"
    if "N-" in case_name:
        return "lop"
    if "pr" in case_name or "ch" in case_name or "rat" in case_name or "kro" in case_name:
        return "tsp"

    print(case_name)
    raise ValueError("case_name->", case_name, "not recognized.")





def order(x):
    return rename_name(x)


print(inst_contr_dict)

test_instances = sorted(test_instances, key=order)


train_instances = test_instances[:]

print(train_instances)

zero_data = np.zeros(shape=(len(train_instances),len(test_instances)))
d = pd.DataFrame(zero_data, columns=test_instances, index=train_instances)

print(inst_contr_dict)

for inst in test_instances:
    for contr in train_instances:
        d.ix[contr, inst] = inst_contr_dict[(inst, contr)]



def inverse(iterable):
    res = np.zeros(len(iterable))
    for i in range(len(iterable)):
        res[iterable[i]] = i
    return res

print(d)




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


# https://stackoverflow.com/Questions/7404116/defining-the-midpoint-of-a-colormap-in-matplotlib
def shiftedColorMap(cmap, start=0, midpoint=0.5, stop=1.0, name='shiftedcmap'):
    '''
    Function to offset the "center" of a colormap. Useful for
    data with a negative min and positive max and you want the
    middle of the colormap's dynamic range to be at zero.

    Input
    -----
      cmap : The matplotlib colormap to be altered
      start : Offset from lowest point in the colormap's range.
          Defaults to 0.0 (no lower offset). Should be between
          0.0 and `midpoint`.
      midpoint : The new center of the colormap. Defaults to
          0.5 (no shift). Should be between 0.0 and 1.0. In
          general, this should be  1 - vmax / (vmax + abs(vmin))
          For example if your data range from -15.0 to +5.0 and
          you want the center of the colormap at 0.0, `midpoint`
          should be set to  1 - 5/(5 + 15)) or 0.75
      stop : Offset from highest point in the colormap's range.
          Defaults to 1.0 (no upper offset). Should be between
          `midpoint` and 1.0.
    '''
    cdict = {
        'red': [],
        'green': [],
        'blue': [],
        'alpha': []
    }

    # regular index to compute the colors
    reg_index = np.linspace(start, stop, 257)

    # shifted index to match the data
    shift_index = np.hstack([
        np.linspace(0.0, midpoint, 128, endpoint=False),
        np.linspace(midpoint, 1.0, 129, endpoint=True)
    ])

    for ri, si in zip(reg_index, shift_index):
        r, g, b, a = cmap(ri)

        cdict['red'].append((si, r, r))
        cdict['green'].append((si, g, g))
        cdict['blue'].append((si, b, b))
        cdict['alpha'].append((si, a, a))

    newcmap = matplotlib.colors.LinearSegmentedColormap(name, cdict)
    plt.register_cmap(cmap=newcmap)

    return newcmap


def transform_name_to_global(name_string):
    return rename_name(name_string)

def average_results(dataframe):
    classes_index = []
    classes_columns = []

    for label in dataframe.index:
        if transform_name_to_global(label) not in classes_index:
            classes_index.append(transform_name_to_global(label))

    for label in dataframe.columns:
        if transform_name_to_global(label) not in classes_columns:
            classes_columns.append(transform_name_to_global(label))


    result = pd.DataFrame(index=classes_index, columns=classes_columns)
    for index_label in classes_index:
        for column_label in classes_columns:
            result.loc[index_label, column_label] = list()

    for index_label in dataframe.index:
        for column_label in dataframe.index:
            if index_label == column_label:
                print("Skipped: ", index_label)
                continue
            result.loc[transform_name_to_global(index_label),
                       transform_name_to_global(column_label)].append(dataframe.loc[index_label, column_label])
    result = result.applymap(mean)

    return result


def save_fig(d, fig_title, fig_path,size_relevant=True, class_relevant=True):

    data = d.copy(deep=True)

    print(data)

    for column in data:
        data[column] -= mean(data[column])
        data[column] /= stdev(data[column]) # max(d[column])

    print(data)

    data = average_results(data)

    print(data)

    yticks =  data.index
    xticks =  data.columns
    data.index = yticks
    data.columns = xticks




    max_val = data.max().max()
    min_val = data.min().min()

    max_reference = 0.6
    min_reference = -1.3

    start = abs(data.min().min() - (min_reference) ) / (abs(min_reference) + max_reference)
    stop = 1 - abs(data.max().max() - (max_reference) ) / (abs(min_reference) + max_reference)

    print(start, stop)

    adjusted_cmap = shiftedColorMap(matplotlib.cm.bwr, midpoint=(1 - max_val / (max_val + abs(min_val))), start=start, stop=stop)

    plt.pcolor(data, cmap=adjusted_cmap)



    FONTSIZE = 15
    plt.yticks(np.arange(0.5, len(data.index), 1), data.index, fontsize=FONTSIZE)
    plt.xticks(np.arange(0.5, len(data.columns), 1), data.columns, rotation = 90,  fontsize=FONTSIZE)
    plt.ylabel("trained on", fontsize=FONTSIZE*1.2)
    plt.xlabel("tested on", fontsize=FONTSIZE*1.2)
    #plt.title("(average - RS) / (BK - RS)")
    #plt.title("normalized, gaussian smoothing, sigma = 0.7")
    #plt.title("rankings on test instances")
    plt.title(" ")
    #plt.title(fig_title)
    plt.colorbar()

    plt.tight_layout()
    plt.savefig(fig_path)
    plt.close()







save_fig(d, "All normalized scores", save_fig_path+"all_norm.pdf") #all
