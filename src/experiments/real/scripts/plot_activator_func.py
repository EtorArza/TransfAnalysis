from matplotlib import pyplot as plt
import numpy as np

SAVEFIG_DIR = "/home/paran/Dropbox/BCAM/04_NEAT_pso_continuous/paper/images/"

x = np.linspace(-5,5,2000)

f_1 = lambda t: np.tanh(t)
f_2 = lambda t: np.tanh(t)**5

plt.plot(x, f_1(x), label="$f(x) = tanh(x)$")
plt.plot(x, f_2(x), label="$f(x) = tanh(x)^5$")
plt.xlabel("$x$")
plt.ylabel("$f(x)$")
plt.legend()
plt.tight_layout()

plt.show()

plt.savefig(SAVEFIG_DIR+"activation_function.pdf")
plt.close()