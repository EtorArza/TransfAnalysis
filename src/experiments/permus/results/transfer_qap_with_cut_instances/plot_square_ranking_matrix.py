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
save_fig_path = "figures/"
#save_fig_path = "/home/paran/Dropbox/BCAM/02_NEAT_permus/paper/images/qap_transfer_cut/"


#input_txt = "result_controllers_GECCO2020_version.txt"
input_txt = "result_controllers_journal_version.txt"
#input_txt = "result_controllers_journal_version_local.txt"



# save in GECCO article dir
import matplotlib
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42




scores = []


with open(input_txt) as f:
    for line in f:
        values = [float(el) for el in line.split("]")[0].strip("[").split(",")]
        n = len(values)
        instance = line.split(",")[n].split(".")[0].split("/")[-1].strip("\"")
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


    new_name = ""


    if fnmatch.fnmatch(case_name, "*tai*a"):
        new_name += "Taixxa_"
    elif fnmatch.fnmatch(case_name, "*sko*"):
        new_name += "Sko_"
    elif fnmatch.fnmatch(case_name, "*tai*b"):
        new_name += "Taixxb_"
    else:
        raise Warning("tai*a, sko or tai*b not found in case name "+str(case_name))

    if "cut30" in case_name:
        new_name += "30"
    elif "cut60" in case_name:
        new_name += "60"
    else:
        raise Warning("cut30 or cut60 not found in case name "+str(case_name))


    if "tai60" in case_name or "72" in case_name:
        new_name+= "_1"
    elif "tai80" in case_name or "81" in case_name:
        new_name+= "_2"
    elif "tai100" in case_name or "90" in case_name:
        new_name+= "_3"
    else:
        raise Warning("Check the naming part, rename_name() func, case_name="+ case_name)


    return new_name


def order(x):
    return rename_name(x)

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


def transform_name_to_global(name_string, type_relevant, size_relevant):
    
    if size_relevant and not type_relevant:
        return name_string.split("_")[1]
    elif type_relevant and not size_relevant:
        return name_string.split("_")[0]
    else:
        raise ValueError("error, (type_relevant XOR size_relevant) == false")


def average_results(dataframe, type_relevant, size_relevant):
    classes_index = []
    classes_columns = []

    for label in dataframe.index:
        if transform_name_to_global(label, type_relevant, size_relevant) not in classes_index:
            classes_index.append(transform_name_to_global(label, type_relevant, size_relevant))

    for label in dataframe.columns:
        if transform_name_to_global(label, type_relevant, size_relevant) not in classes_columns:
            classes_columns.append(transform_name_to_global(label, type_relevant, size_relevant))


    result = pd.DataFrame(index=classes_index, columns=classes_columns)
    for index_label in classes_index:
        for column_label in classes_columns:
            result.loc[index_label, column_label] = list()

    for index_label in dataframe.index:
        for column_label in dataframe.index:
            if index_label == column_label:
                print("Skipped: ", index_label)
                continue
            result.loc[transform_name_to_global(index_label, type_relevant, size_relevant), transform_name_to_global(column_label, type_relevant, size_relevant)].append(dataframe.loc[index_label, column_label])
    result = result.applymap(mean)

    return result


def save_fig(d, fig_title, fig_path, class_relevant, size_relevant):

    data = d.copy(deep=True)

    for column in data:
        data[column] -= mean(data[column])
        data[column] /= stdev(data[column]) # max(d[column])


    # for i in range(d.shape[0]):
    #    d.iloc[i,:] -= mean(d.iloc[i,:])
    #    d.iloc[i,:] /= stdev(d.iloc[i,:])


    yticks = [rename_name(el,size_relevant, class_relevant) for el in data.index]
    xticks = [rename_name(el,size_relevant, class_relevant) for el in data.columns]
    data.index = yticks
    data.columns = xticks

    #data = average_results(data, class_relevant, size_relevant)

    print(data)

    max_val = data.max().max()
    min_val = data.min().min()

    max_reference = 1.3
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


all_instances.sort(key=order, reverse=True)

small_instances = [ins for ins in all_instances if "cut30" in ins]
big_instances = [ins for ins in all_instances if "cut60" in ins]


taia_instances = [ins for ins in all_instances if "0a" in ins]
taib_instances = [ins for ins in all_instances if "0b" in ins]
sko_instances = [ins for ins in all_instances if "sko" in ins]





#save_fig(d, "All normalized scores", save_fig_path+"all_norm.pdf", True, True) #all
save_fig(d.loc[small_instances,small_instances], "Small instances", save_fig_path+"small.pdf", True, False) # small instances
save_fig(d.loc[big_instances,big_instances], "Large instances", save_fig_path+"large.pdf", True, False) # large instances
save_fig(d.loc[taia_instances,taia_instances], "taia_instances", save_fig_path+"taia_instances.pdf", False, True) # taia instances
save_fig(d.loc[taib_instances,taib_instances], "taib_instances", save_fig_path+"taib_instances.pdf", False, True) # taib instances
save_fig(d.loc[sko_instances,sko_instances], "sko_instances", save_fig_path+"sko_instances.pdf", False, True) # sko instances
