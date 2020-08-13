

from __future__ import print_function
import os
import neat
import numpy as np 
from random import seed
from numba import njit
from statistics import mean
from multiprocessing import Pool
from tqdm import tqdm
from joblib import Parallel, delayed
import pickle
from celluloid import Camera
from matplotlib import pyplot as plt
import sklearn.manifold

seed(2)

PLOT_PARTICLE_SIZE = 6



# animation function.  This is called sequentially
last_emb = []
best_emb = []



@njit
def dist_to_distances_matrix(D_matrix, D_vector):
    min_dist = 1000.0

    for i in range(len(D_matrix)):
        d = np.sqrt(np.sum((D_matrix[i] - D_vector)**2))
        if d < min_dist:
            min_dist = d
    return d





def convert_inidces_to_colors(sigma, m):
    offset = 1.5
    res = ((m-1) - np.array([sigma.index(i)  for i in range(len(sigma))])) / (m-1) / offset
    res[sigma[0]] = 1.0
    return res


def sizes_according_to_reps(pop_matrix, m):
    transposed_m = pop_matrix.transpose()
    sizes = np.zeros(m)
    counter = 0
    eps = 10**-5
    for i in range(len(transposed_m)):
        for j in range(len(transposed_m)):
            if np.sum(np.abs(transposed_m[i] - transposed_m[j])) < eps:
                counter += 1
        sizes[i] = counter*PLOT_PARTICLE_SIZE
        counter = 0
    return sizes 
    

def order_matrix_according_to_list_order(matrix, order_list):
    tuple_list = list(zip(matrix, order_list))
    tuple_list.sort(key=lambda x: x[1])
    res =  np.array([el[0] for el in tuple_list])
    order_list.sort()
    return res


if __name__ == '__main__':


    # First set up the figure, the axis, and the plot element we want to animate
    fig = plt.figure()
    ax = plt.axes(xlim=(-1, 1), ylim=(-1, 1))
    camera = Camera(fig)

    positions = []

    with open("positions.txt", "r") as file:
        for line in file:
            positions.append(eval(line.strip()))

    n_frames = len(positions)
    for i in tqdm(range(min(n_frames, 100))):
        # best_known = trans_data[:,0]
        # trans_data = trans_data[:,1:]
        line = positions[i]
        trans_data = np.array(line)
        colors = convert_inidces_to_colors(list(range(len(line))), len(line))
        trans_data = order_matrix_according_to_list_order(trans_data, colors).transpose()
        

        # for s in range(m):
        #     if (pop_list[i][-1]==pop_list[i][0][s]).all():
        #         print("---")
        #         best_emb = trans_data[:,0].copy()
        #         break

        #print(trans_data)

            
        sizes = sizes_according_to_reps(trans_data, len(line))
        # print(trans_data)
        # print(trans_data.shape)
        plt.scatter(*trans_data, c=colors, s=sizes, cmap="viridis", )
        plt.xlim(0, 1)
        plt.ylim(0, 1)
        # print(best_emb)
        camera.snap()
    animation = camera.animate()
    animation.save('animation.mp4', fps=5)



    print("animation saved")

