import matplotlib.pyplot as plt
import subprocess
import numpy as np
from tqdm import tqdm as tqdm
from matplotlib.ticker import StrMethodFormatter
from joblib import Parallel, delayed

savefig_path = "experimentResults/transfer_16_continuous_problems/results/figures/"
subprocess.run(f"mkdir -p {savefig_path}", shell=True) # write out into log.txt


def evaluate_cont_problem(problem_index, x, y):
    exec_res=subprocess.run(f"./neat -evaluate-continuous-problem {problem_index} {x},{y}",shell=True, capture_output=True)  
    return str(exec_res.stdout).strip("'b")

def eval_cont_mesh(problem_index, X, Y):
    z = np.vectorize(lambda x,y: evaluate_cont_problem(problem_index, x, y))(X.flatten(),Y.flatten())
    z = z.astype(float)
    return np.reshape(z, X.shape)

def generate_contour_plot(problem_index):


    # https://andrewpwheeler.com/2021/01/29/filled-contour-plot-in-python/
    n = 80
    #np.logspace(0,np.log10(10000),n) #if you want to do logged
    X, Y = np.meshgrid(np.linspace(0,1,n), np.linspace(0,1,n))
    Z = eval_cont_mesh(problem_index, X, Y)
    Z = Z[:-1, :-1]

    fig, ax = plt.subplots()
    CS = ax.pcolormesh(X, Y, Z, cmap='RdPu', edgecolors="face")
    clb = fig.colorbar(CS)
    #clb.ax.set_xlabel('Revenue') #Abit too wide
    clb.ax.set_title('f') #html does not like the dollar sign
    ax.set_xlabel('$x_1$')
    ax.set_ylabel('$x_2$')
    #ax.yaxis.set_major_formatter(StrMethodFormatter('${x:,.0f}'))
    plt.title(f'Continuous function {problem_index}')
    plt.xticks(np.arange(0,1,0.1))
    plt.yticks(np.arange(0,1,0.1))
    plt.savefig(savefig_path+f'Contour_prblem_{problem_index}.pdf',dpi=300,bbox_inches='tight')
    plt.close()


Parallel(n_jobs=6, verbose=12)(delayed(generate_contour_plot)(i) for i in range(0,13))

