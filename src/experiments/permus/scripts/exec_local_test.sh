


if [[ "$#" -ne 6  ]] ; then
    echo 'Please provide the name of the problem, the name of the instance and the path to the controller, MAX_SOLVER_TIME, and the location of output and wether to record response or not. $# parameters where provided. 4 are needed. Example: '
    echo ""
    echo 'script.sh qap tai35a.dat.dat experiment_results/inter_instance_transfer/qap_tai35a/experiment_1/fittest_1 0.5 results.txt false'
    echo ""
    echo 'Exitting...'
    exit 1
fi




cat > tmp.ini <<EOF
; config file for test in hpc hipatia


[Global] 
MODE = test ;
PROBLEM_NAME = permu


THREADS = 3 ;
N_EVALS = 2 ;
N_REPS = 20 ;
CONTROLLER_PATH = $3 ; 
COMPUTE_RESPONSE = $6

MAX_SOLVER_TIME = $4 ; 
POPSIZE = 20 ;
TABU_LENGTH = 40 ;
N_EVALS = 500 ;
N_REPS = 2 ;

PROBLEM_TYPE = $1 ; 
PROBLEM_PATH = $2 ; 

EOF



./neat "tmp.ini"


cat "result.txt" >> "$5"
rm "result.txt"

echo $2


# #end
