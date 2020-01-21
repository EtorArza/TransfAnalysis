
from matplotlib import pyplot as plt
import sys



if len(sys.argv) < 2:
    print("Input file not provided. Exiting...")
    exit(1)

filename = sys.argv[1]

lines = []
with open(filename) as f:
    for line in f:
        lines.append(line.strip("\n"))

fitnesses_1 = []
fitnesses_2 = []
last_fitness_1 = -10
last_fitness_2 = -10
for line in lines:
    if "[BEST_FITNESS_IMPROVED] --> " in line:
        last_fitness_1 = -float(line.split("-->  ")[1].split(" ")[0])
        last_fitness_2 = -float(line.split("-->  ")[1].split(" ")[1])
    if "fitness_array" in line:
        fitnesses_1.append(last_fitness_1)
        fitnesses_2.append(last_fitness_2)
        print(last_fitness_1, last_fitness_2)

plt.plot(fitnesses_1)
#plt.plot(fitnesses_2)
plt.xlabel("Iteration")
plt.ylabel("Fitness")

plt.savefig("convergence_neat.pdf")

