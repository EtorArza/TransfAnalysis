from matplotlib import pyplot as plt


with open("result_popsize_experiment.txt") as f:
    line_list_list = []
    for line in f:
        line = line.strip()
        line_list_list += [eval(line)]

def extract_popsize(line_list):
    res = line_list[2]
    res = res.split("/")[-1]
    res = res.split("popsize")[-1]
    res = res.split("_best.controller")[0]
    return int(res)

labels = []
values = []
for line_list in line_list_list:
    labels.append(str(extract_popsize(line_list)))
    values += [line_list[0]]


# Create a figure instance
fig = plt.figure(1, figsize=(9, 6))

# Create an axes instance
ax = fig.add_subplot(111)

# Create the boxplot
bp = ax.boxplot(values, labels=labels)

# Save the figure
fig.savefig('res.pdf', bbox_inches='tight')