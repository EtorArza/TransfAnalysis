FILE_PATHS=()

FILE_PATHS+=("experimentScripts/transfer_permus.sh")
FILE_PATHS+=("src/experiments/permus_multi/scripts/hip_train_multi_array.sl")
FILE_PATHS+=("src/experiments/permus/scripts/hip_test_array.sl")

FILE_PATHS+=("experimentScripts/transfer_16_continuous_problems.sh")
FILE_PATHS+=("src/experiments/real/scripts/hip_test_array_in_one_of_16_problems.sl")
FILE_PATHS+=("src/experiments/real/scripts/hip_train_array_16_continuous.sl")

FILE_PATHS+=("experimentScripts/transfer_experiment_generated.sh")
FILE_PATHS+=("src/experiments/real/scripts/hip_test_array_randomly_generated.sl")
FILE_PATHS+=("src/experiments/real/scripts/hip_train_array_random_problem.sl")


for FILE in ${FILE_PATHS[@]};
do

    sed -i 's/--mem=32G/--mem=4G/' $FILE
    sed -i 's/--mem=16G/--mem=4G/' $FILE
    sed -i 's/NEAT_POPSIZE=500/NEAT_POPSIZE=8/g' $FILE
    sed -i 's/NEAT_POPSIZE=1000/NEAT_POPSIZE=8/g' $FILE
    sed -i 's/MAX_TRAIN_ITERATIONS=2000/MAX_TRAIN_ITERATIONS=2/g' $FILE
    sed -i 's/MAX_TRAIN_ITERATIONS=1000/MAX_TRAIN_ITERATIONS=2/g' $FILE
    sed -i 's/MAX_TRAIN_ITERATIONS=10000/MAX_TRAIN_ITERATIONS=2/g' $FILE
    sed -i 's/MAX_TRAIN_ITERATIONS=5000/MAX_TRAIN_ITERATIONS=2/g' $FILE
    sed -i 's/MAX_SOLVER_FE=20000/MAX_SOLVER_FE=40/g' $FILE
    sed -i 's/MAX_SOLVER_FE=4000/MAX_SOLVER_FE=40/g' $FILE
    sed -i 's/MAX_SOLVER_FE=1000/MAX_SOLVER_FE=40/g' $FILE
    sed -i 's/--cpus-per-task=4/--cpus-per-task=2/g' $FILE
    sed -i 's/--cpus-per-task=8/--cpus-per-task=2/g' $FILE
    sed -i 's/--cpus-per-task=16/--cpus-per-task=2/g' $FILE
    sed -i 's/--cpus-per-task=32/--cpus-per-task=2/g' $FILE
    sed -i 's/--time=30-00:00:00/--time=00-00:30:00/g' $FILE
    sed -i 's/--time=0-06:00:00/--time=00-00:30:00/g' $FILE
    sed -i 's/--time=1-00:00:00/--time=00-00:30:00/g' $FILE
    sed -i 's/--time=1-0:00:00/--time=00-00:30:00/g' $FILE
    sed -i 's/--time=5-00:00:00/--time=00-00:30:00/g' $FILE
    sed -i 's/--time=5-0:00:00/--time=00-00:30:00/g' $FILE
    sed -i 's/#SBATCH -p xlarge/#SBATCH -p short/g' $FILE
    sed -i 's/#SBATCH -p medium/#SBATCH -p short/g' $FILE
    sed -i 's/#SBATCH -p large/#SBATCH -p short/g' $FILE
    sed -i 's/N_EVALS=10000/N_EVALS=10/g' $FILE
done
