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
save_fig_path = "src/experiments/permus_multi/results/figures/"
#save_fig_path = "/home/paran/Dropbox/BCAM/02_NEAT_permus/paper/images/qap_transfer_cut/"


input_txt_1 = "src/experiments/permus/results/transfer_qap_with_cut_instances/result_controllers_journal_version.txt"
input_txt_2 = "src/experiments/permus_multi/results/result_controllers_cut_qap_multi_on_journal_version.txt"



# save in GECCO article dir
import matplotlib
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42




scores = []

for input_txt in [input_txt_1, input_txt_2]:
    with open(input_txt) as f:
        for line in f:
            values = [float(el) for el in line.split("]")[0].strip("[").split(",")]
            n = len(values)
            instance = line.split(",")[n].split(".")[0].split("/")[-1]
            controller = line.split(",")[n+1].split("/")[-1].split("_gen_")[0].split("with_")[1]
            scores.append([values, controller, instance])



inst_contr_dict = dict()
test_instances = []
train_instances = []
for el in scores:
    if el[1] not in train_instances:
        train_instances.append(el[1])
    if el[2] not in test_instances:
        test_instances.append(el[1])
    #inst_contr_dict[(el[1],el[2])] = (mean(el[0]) - RS_res) / (BK - RS_res)
    inst_contr_dict[(el[1],el[2])] = mean(el[0])




def xor(a, b):
    return (a and not b) or (not a and b)



def rename_name(case_name_input, size_relevant=True, class_relevant=True):


    new_name = ""

    if len(case_name_input.split("_")) == 2:
        case_names = [case_name_input]

    elif len(case_name_input.split("_")) == 4:
        case_name_splitted = case_name_input.split("_")
        case_names = ["_".join(case_name_splitted[:2]), "_".join(case_name_splitted[2:])]

    elif len(case_name_input.split("_")) == 6:
        case_name_splitted = case_name_input.split("_")
        case_names = ["_".join(case_name_splitted[:2]), "_".join(case_name_splitted[2:4]), "_".join(case_name_splitted[4:6])]
    else:
        raise ValueError("Error, not recognized case_name_input:", case_name_input)

    for case_name in case_names:

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
            new_name+= "_1_"
        elif "tai80" in case_name or "81" in case_name:
            new_name+= "_2_"
        elif "tai100" in case_name or "90" in case_name:
            new_name+= "_3_"
        else:
            raise Warning("Check the naming part, rename_name() func, case_name="+ case_name)


    return new_name


def order(x):
    return "z"*len(x) + rename_name(x)

def order_len(x):
    return "z"*len(x) + x

train_instances = sorted(list(set(train_instances)), key=order)
test_instances = sorted(list(set(test_instances)), key=order)



zero_data = np.zeros(shape=(len(train_instances),len(test_instances)))
d = pd.DataFrame(zero_data, columns=test_instances, index=train_instances)

count = 0
for inst in test_instances:
    for contr in train_instances:
        count += 1
        print([contr, inst], count)
        d.ix[contr, inst] = inst_contr_dict[(contr, inst)]



def inverse(iterable):
    res = np.zeros(len(iterable))
    for i in range(len(iterable)):
        res[iterable[i]] = i
    return res

print(d)


n = d.shape[0]
m = d.shape[1]





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
        name_string = name_string.replace("Sko_", "")
        name_string = name_string.replace("Taixxa_", "")
        name_string = name_string.replace("Taixxb_", "")
    elif type_relevant and not size_relevant:
        name_string = name_string.replace("_30", "")
        name_string = name_string.replace("_60", "")
    else:
        raise ValueError("error, (type_relevant XOR size_relevant) == false")
    name_string = name_string.replace("_3_", "_")
    name_string = name_string.replace("_2_", "_")
    name_string = name_string.replace("_1_", "_")
    return name_string

def average_results(dataframe, type_relevant, size_relevant):
    classes_index = []
    classes_columns = []

    for label in dataframe.index:
        if transform_name_to_global(label, type_relevant, size_relevant) not in classes_index:
            classes_index.append(transform_name_to_global(label, type_relevant, size_relevant))

    for label in dataframe.columns:
        if transform_name_to_global(label, type_relevant, size_relevant) not in classes_columns:
            classes_columns.append(transform_name_to_global(label, type_relevant, size_relevant))

    classes_index = sorted(list(classes_index), key=order_len, reverse = True)
    classes_columns = sorted(list(classes_columns), key=order_len, reverse = True)

    result = pd.DataFrame(index=classes_index, columns=classes_columns)
    for index_label in classes_index:
        for column_label in classes_columns:
            result.loc[index_label, column_label] = list()

    for index_label in dataframe.index:
        for column_label in dataframe.columns:
            #if index_label == column_label:
            #    print("Skipped: ", index_label)
            #    continue

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

    data = average_results(data, class_relevant, size_relevant)



    max_val = data.max().max()
    min_val = data.min().min()

    max_reference = 2.0
    min_reference = -2.0

    start = abs(data.min().min() - (min_reference) ) / (abs(min_reference) + max_reference)
    stop = 1 - abs(data.max().max() - (max_reference) ) / (abs(min_reference) + max_reference)


    adjusted_cmap = shiftedColorMap(matplotlib.cm.bwr, midpoint=(1 - max_val / (max_val + abs(min_val))))

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








small_instances_train = [ins for ins in list(set(train_instances)) if "cut60" not in ins]
small_instances_test = [ins for ins in list(set(test_instances)) if "cut60" not in ins]


big_instances_train = [ins for ins in list(set(train_instances)) if "cut30" not in ins]
big_instances_test = [ins for ins in list(set(test_instances)) if "cut30" not in ins]


taia_instances_train = [ins for ins in list(set(train_instances)) if "0b" not in ins and "sko" not in ins]
taia_instances_test = [ins for ins in list(set(test_instances)) if "0b" not in ins and "sko" not in ins]

taib_instances_train = [ins for ins in list(set(train_instances)) if "0a" not in ins and "sko" not in ins]
taib_instances_test = [ins for ins in list(set(test_instances)) if "0a" not in ins and "sko" not in ins]


sko_instances_train = [ins for ins in list(set(train_instances)) if "0a" not in ins and "0b" not in ins]
sko_instances_test = [ins for ins in list(set(test_instances)) if "0a" not in ins and "0b" not in ins]





#save_fig(d, "All normalized scores", save_fig_path+"all_norm.pdf", True, True) #all
save_fig(d.loc[small_instances_train,small_instances_test], "Small instances", save_fig_path+"small.pdf", True, False) # small instances
save_fig(d.loc[big_instances_train,big_instances_test], "Large instances", save_fig_path+"large.pdf", True, False) # large instances
save_fig(d.loc[taia_instances_train,taia_instances_test], "taia_instances", save_fig_path+"taia_instances.pdf", False, True) # taia instances
save_fig(d.loc[taib_instances_train,taib_instances_test], "taib_instances", save_fig_path+"taib_instances.pdf", False, True) # taib instances
save_fig(d.loc[sko_instances_train,sko_instances_test], "sko_instances", save_fig_path+"sko_instances.pdf", False, True) # sko instances
