import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d 
import subprocess
from tqdm import tqdm as tqdm
from joblib import Parallel, delayed

N_INTERVALS = 200
x_lower_lim = 0
x_upper_lim = 1


def grienhawk(x_vec):
    val_1 = 0.0
    for i in range(len(x_vec)):
        val_1 += x_vec[i] * x_vec[i] / 4000.0

    val_2 = 1.0

    for i in range(1,len(x_vec)+1):
        val_2 *= np.cos(x_vec[i-1] / np.sqrt(float(i)))
    
    res = 1.0 + val_1 - val_2

    return res


def random_quadratic(x_vec):
    res = subprocess.run("./neat {} {}".format(*x_vec),shell=True,capture_output=True)
    return float(res.stdout.strip())


x = np.arange(x_lower_lim, x_upper_lim, (x_upper_lim - x_lower_lim) / N_INTERVALS)
y = np.arange(x_lower_lim, x_upper_lim, (x_upper_lim - x_lower_lim) / N_INTERVALS)

xx, yy = np.meshgrid(x, y)

xx = xx.flatten()
yy = yy.flatten()
print(xx)
print(np.stack((xx,yy)).T)


list_of_rows = Parallel(n_jobs=16)(delayed(random_quadratic)(item) for item in tqdm(np.stack((xx,yy)).T))
z = np.array(list_of_rows) + 1

print(len(xx), len(yy), len(z))

xx = xx.reshape((N_INTERVALS, N_INTERVALS)).T
yy = xx.reshape((N_INTERVALS, N_INTERVALS)).T
z = z.reshape((N_INTERVALS, N_INTERVALS)).T

print(xx)

# Creating figure 
fig = plt.figure(figsize = (10, 7)) 
ax = plt.axes(projection ="3d") 
  
# Creating plot 
ax.plot_surface(xx, yy, z, cmap = "viridis", rasterized = True); 
plt.xlabel("")
plt.ylabel("")
ax.set_zticks([i for i in range(7)])
ax.set_zlim([0,6])
plt.title("") 
  
# show plot 
ax.view_init(60, 45)

plt.savefig("src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/random_test_function.pdf", dpi = 600)
plt.close()
