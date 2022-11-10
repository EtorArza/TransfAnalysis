import numpy as np
from matplotlib import pyplot as plt
from matplotlib.animation import FuncAnimation
from tqdm import tqdm as tqdm
import adapt_to_resources


plt.style.use('seaborn-pastel')



adapt_to_resources.SOLVER_POPSIZE=10
adapt_to_resources.MAX_SOLVER_FE=1000
adapt_to_resources.N_EVALS=1
adapt_to_resources.N_EVALS_TEST=1
adapt_to_resources.PROBLEM_DIM=2
adapt_to_resources.THREADS=1



def write_progress_compatible_pb(pb, current_frame, total_frames):
    pb.total = total_frames*1000
    pb.refresh()
    pb.update(current_frame)

if __name__ == "__main__":

    pb = tqdm()


    controller_path = "experimentResults/transfer_16_continuous_problems/controllers/top_controllers/TrainOnlyInF_1_best.controller"
    PROBLEM_INDEX = 1

    adapt_to_resources.run_with_controller_continuous(PROBLEM_INDEX, controller_path, PRINT_POSITIONS=True)

    positions = []
    with open("positions.txt", "r") as f:
        for line in f:
            position_array = np.array(eval(line.strip())).T
            positions.append(position_array)


    print(positions[0:2])

    fig = plt.figure()
    ax = plt.axes(xlim=(0, 1), ylim=(0, 1))
    line, = ax.plot(positions[0][0], positions[0][1], "bo")



    def init():
        line.set_data(positions[0][0], positions[0][1])
        return line,
    def animate(i):
        x = positions[i][0]
        y = positions[i][1]
        line.set_data(x, y)
        return line,



    anim = FuncAnimation(fig, animate, init_func=init, frames=len(positions), blit=True)
    anim.save(f'animation_{PROBLEM_INDEX}_{controller_path.split("/")[-1].split(".")[0]}.mp4', fps=2, extra_args=['-vcodec', 'libx264'], progress_callback=lambda x,y: write_progress_compatible_pb(pb,x,y))

