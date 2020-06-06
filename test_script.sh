for PROBLEM_PATH in "src/experiments/permus/instances/transfer_qap_cut_instances/"*; do
    i=$((i+1))

    CONTROLLER_NAME_PREFIX=`basename ${PROBLEM_PATH}`
    echo ${CONTROLLER_NAME_PREFIX}
    echo "huh"

    PROBLEM_TYPE_ARRAY+=("qap")
    PROBLEM_PATH_ARRAY+=("${PROBLEM_PATH}")
    POPSIZE_ARRAY+=("128")
    CONTROLLER_NAME_PREFIX_ARRAY+=("${CONTROLLER_NAME_PREFIX}")
    EXPERIMENT_FOLDER_NAME_ARRAY+=("${SRCDIR}/${EXPERIMENT_FOLDER_NAME}")
    CONTROLLER_ARRAY+=("${EXPERIMENT_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
    SEED_ARRAY+=("2")
    MAX_SOLVER_TIME_ARRAY+=("0.25")
done
