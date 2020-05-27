from matplotlib import pyplot as plt

RESULT_TXT = "result_popsize_experiment.txt"

with open(RESULT_TXT) as f:
    line_list_list = []
    for line in f:
        line = line.strip()
        line_list_list += [eval(line)]

def extract_popsize(line_list):
    res = line_list[2]
    res = res.split("/")[-1]
    res = res.split("popsize")[-1]
    res = res.split("_SEED")[0]
    return int(res)

labels = []
values = []
for line_list in line_list_list:
    current_label = int(extract_popsize(line_list))
    # if line_list[0][0] < -100000:
    #     continue
    if current_label not in labels:
        labels.append(current_label)
        values += [line_list[0]]
    else:
        values[labels.index(current_label)].append(line_list[0][0])


labels, values = (list(t) for t in zip(*sorted(zip(labels, values)))) # sort

print(labels)

print(values)

# Create a figure instance
fig = plt.figure(1, figsize=(9, 6))

# Create an axes instance
ax = fig.add_subplot(111)

# Create the boxplot
bp = ax.boxplot(values, labels=labels)

# Save the figure
fig.savefig('res.pdf', bbox_inches='tight')