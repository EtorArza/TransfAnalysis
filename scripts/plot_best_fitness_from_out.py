from matplotlib import pyplot as plt
import sys


optimum = - 2422002

if len(sys.argv) < 2:
	print("Input file not provided. Exiting...")
	exit(1)

filename = sys.argv[1]

lines = []
with open(filename) as f:
	for line in f:
		lines.append(line.strip("\n"))

fitnesses = []
for line in lines:
	if "fittest " in line and "]:" in line:
		fitness = float(line.split(" ")[2][8:-1])
		fitnesses.append(fitness)
		print(fitness)

plt.plot(fitnesses)
plt.xlabel("Iteration")
plt.ylabel("Fitness")

#plt.axhline(optimum)

plt.savefig("convergence_neat.pdf")