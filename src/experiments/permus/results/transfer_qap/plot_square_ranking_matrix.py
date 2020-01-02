from statistics import mean, variance, stdev, median
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from seaborn import clustermap
from scipy.ndimage.filters import gaussian_filter
from copy import deepcopy
import random


scores = []

with open("result_controllers.txt") as f:
    for line in f:
        values = [float(el) for el in line.split("]")[0].strip("[").split(",")]
        n = len(values)
        instance = line.split(",")[n].split(".")[0].split("/")[-1]
        controller = line.split(",")[n+1].split("/")[-1].split("_")[0]
        scores.append([values, instance, controller])

RS_values = dict()
with open("results_RandomSearch.txt") as f:
    for line in f:
        values = [float(el) for el in line.split("]")[0].strip("[").split(",")]
        n = len(values)
        instance = line.split(",")[n].split(".")[0].split("/")[-1]
        controller = line.split(",")[n+1].split("/")[-1].split("_")[0]
        RS_values[instance] = mean(values)


BK_values = {'Bwil50': - 48816, 'Alipa60a': -107218, 'Alipa60b': -2520135, 'Ctai80b': -818415043,
'Bwil100': -273038, 'Bsko72':-66256, 'Atai60a': -7205962, 'Atai50a': -4938796, 'Bsko56': -34458,
'Atai80a': -13515450, 'Cchr22b': -6194, 'Bsko64':-48498, 'Alipa70b': -4603200, 'Cchr25a': -3796,
'Btho150': -8133398, 'Alipa70a': -169755, 'Ctai60b': -608215054, 'Ctai50b': -458821517, 'Ctai100b': -1185996137}




inst_contr_dict = dict()
test_instances = []
for el in scores:
    if el[1] not in test_instances:
        test_instances.append(el[1])
    BK = BK_values[el[1]]
    RS_res = RS_values[el[1]]
    #inst_contr_dict[(el[1],el[2])] = (mean(el[0]) - RS_res) / (BK - RS_res)
    inst_contr_dict[(el[1],el[2])] = mean(el[0])




def xor(a, b):
    return (a and not b) or (not a and b)



def order(x):
    if "lipa" in x:
        if "b" in x:
            return "Alipa1" + x
    
    return x


test_instances = sorted(test_instances, key=order)



train_instances = [
"tai50a", "tai60a", "tai80a",
"sko56", "sko64", "sko72",
"tai50b", "tai60b", "tai80b",
]



zero_data = np.zeros(shape=(len(train_instances),len(test_instances)))
d = pd.DataFrame(zero_data, columns=test_instances, index=train_instances)



for inst in test_instances:
    for contr in train_instances:
        d.ix[contr, inst] = inst_contr_dict[(inst, contr)]

print(d)



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

print(d.shape)

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

print(get_loss(d))

plt.pcolor(d, )
plt.yticks(np.arange(0.5, len(d.index), 1), d.index)
plt.xticks(np.arange(0.5, len(d.columns), 1), d.columns, rotation = 90)
plt.ylabel("trained with")
plt.xlabel("tested on")
#plt.title("(average - RS) / (BK - RS)")
#plt.title("normalized, gaussian smoothing, sigma = 0.7")
#plt.title("rankings on test instances")
plt.title("rankings on test instances, reordered minimizing column distance")
plt.tight_layout()

plt.show()