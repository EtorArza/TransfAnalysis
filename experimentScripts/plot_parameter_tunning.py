import pandas as pd
import numpy as np
from matplotlib import pyplot as plt

def find_between(s, start, end): # find substring between two strings
    return (s.split(start))[1].split(end)[0]

for domain in ["continuous", "permus"]:

    rows = []
    with open("experimentResults/hyperparameter_tunning/results/score.txt") as file:
        for line in file:
            if domain not in line:
                continue
            line = eval(line)
            f = line[0][0]
            seed = int(find_between(line[2], "_seed", "_best."))
            popsize = int(find_between(line[2], "popsize_", "_seed"))
            rows.append([popsize, seed, f])


    df = pd.DataFrame(rows, columns=["popsize","seed","f"])

    fig = df.boxplot(column="f", by="popsize")
    plt.show()
    plt.close()
