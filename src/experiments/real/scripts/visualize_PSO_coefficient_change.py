import numpy as np
from matplotlib import pyplot as plt



file_path = "detailed_response.txt"
save_path = "/home/paran/Dropbox/BCAM/04_NEAT_pso_continuous/paper/images/"

def read_matrices(filen_path):
    result = []
    with open(file_path) as f:
        for line in f:
            line = line.strip()
            if "i" not in line:
                continue
            i = int(line.split(" = ")[-1].split(" ( ")[0])
            if i==0:
                result.append([])
        
            values = line.split("( ")[-1].split(" )")[0].split(" ")
            values = [float(el) for el in values]
            result[-1].append(values)
    for i, _ in enumerate(result):
        result[i] = np.array(result[i])
    return result

x_labels = ["solution rank", "relative time"]

datasets = read_matrices(file_path)

for i, data in enumerate(datasets):
    x_label = x_labels[i]
    plt.plot(np.array(range(1, data.shape[0]+1)), data[:,0], label="Momentum")
    plt.plot(np.array(range(1, data.shape[0]+1)), data[:,1], label="GBest")
    plt.plot(np.array(range(1, data.shape[0]+1)), data[:,2], label="LBest")
    plt.xlabel(x_label)
    plt.ylim((-1,1))
    plt.legend()
    plt.tight_layout()
    plt.savefig(save_path+x_label+"_detailedoutput.pdf")
    plt.close()