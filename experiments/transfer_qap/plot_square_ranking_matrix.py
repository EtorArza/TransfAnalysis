from statistics import mean, variance, stdev, median
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from seaborn import clustermap

scores = []

with open("result_controllers.txt") as f:
    for line in f:
        values = [float(el) for el in line.split("]")[0].strip("[").split(",")]
        n = len(values)
        instance = line.split(",")[n].split(".")[0].split("/")[-1]
        controller = line.split(",")[n+1].split("/")[-1].split("_")[0]
        scores.append([values, instance, controller])

#print(scores)

inst_contr_dict = dict()

instances = []
for el in scores:
    if el[1] not in instances:
        instances.append(el[1])
    inst_contr_dict[(el[1],el[2])] = median(el[0])



instances = ["dre42", "dre56", "dre72", 
"lipa50a", "lipa60a", "lipa70a", 
"tai40a", "tai50a", "tai60a",
"lipa50b", "lipa60b", "lipa70b",
"tai40b", "tai50b", "tai60b",
]
 


zero_data = np.zeros(shape=(len(instances),len(instances)))
d = pd.DataFrame(zero_data, columns=instances, index=instances)

for inst in instances:
    for contr in instances:
        d.ix[contr, inst] = inst_contr_dict[(inst, contr)]

print(d)



def inverse(iterable):
    res = np.zeros(len(iterable))
    for i in range(len(iterable)):
        res[iterable[i]] = i
    return res


for column in d:
#    d[column] = inverse(np.argsort(d[column]))
    d[column] -= mean(d[column])
    d[column] /= max(d[column])# stdev(d[column])

print(d)


plt.pcolor(d, )
plt.yticks(np.arange(0.5, len(d.index), 1), d.index)
plt.xticks(np.arange(0.5, len(d.columns), 1), d.columns, rotation = 90)
plt.ylabel("trained with")
plt.xlabel("tested on")
plt.tight_layout()

plt.show()

# from scipy.cluster.hierarchy import linkage
# link = linkage(d, optimal_ordering=True) # D being the measurement
# g = clustermap(d, row_linkage=link, col_linkage=link)
# g.savefig("res.pdf")