a = """tai35a.dat.dat experimentation_results/qap_incremental_to_10s/controllers/fittest_700 0.6641200131131189 0.15
tai35a.dat.dat experimentation_results/qap_incremental_to_10s/controllers/fittest_800 0.4245248352396076 0.15
tai35a.dat.dat experimentation_results/qap_incremental_to_10s/controllers/fittest_200 0.47060241899057065 0.15
tai35a.dat.dat experimentation_results/qap_incremental_to_10s/controllers/fittest_500 0.4915768029919051 0.15
tai35a.dat.dat experimentation_results/qap_incremental_to_10s/controllers/fittest_100 1.8454154868575665 0.15
tai35a.dat.dat experimentation_results/qap_incremental_to_10s/controllers/fittest_1000 0.3765479962444292 0.15
tai35a.dat.dat experimentation_results/qap_incremental_to_10s/controllers/fittest_900 0.4605281085647328 0.15
tai35a.dat.dat experimentation_results/qap_incremental_to_10s/controllers/fittest_300 0.5341036051993351 0.15
tai35a.dat.dat experimentation_results/qap_incremental_to_10s/controllers/fittest_1300 0.3931045473950889 0.15
tai35a.dat.dat experimentation_results/qap_incremental_to_10s/controllers/fittest_400 0.5196527500803054 0.15
tai35a.dat.dat experimentation_results/qap_incremental_to_10s/controllers/fittest_600 0.5258459737027468 0.15
tai35a.dat.dat experimentation_results/qap_incremental_to_10s/controllers/fittest_1100 0.4651523822028223 0.15
tai35a.dat.dat experimentation_results/qap_incremental_to_10s/controllers/fittest_1200 0.44805908500488445 0.15
tai35a.dat.dat experimentation_results/qap_incremental_to_10s/controllers/fittest_1400 0.4424026074297214 0.15"""

from matplotlib import pyplot as plt

b = a.split("\n")

b.sort(key = lambda x: x[1])

x = []
y = []
gen = 100
for line in b:
	val = float(line.split(" ")[2])
	x.append(gen)
	y.append(val)
	gen += 100

plt.plot(x,y)
plt.savefig("convergence_neat.pdf")

print(b)