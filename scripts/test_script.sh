


source scripts/array_to_string_functions.sh 


array_var="A,B|C,D"

replaced_with="xyz_comma_xyz"
array_var=${array_var//","/$replaced_with}

TRAINING_JOB_ID=`sbatch --parsable --export=array_var="${array_var}" --array=0-1 test_script.sl`


first="Suzy, Suzy, Suzy"
second="Sara"
first=${first//Suzy/$second}
# first is now "Sara, Sara, Sara"