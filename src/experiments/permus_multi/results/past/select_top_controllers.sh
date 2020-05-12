ALL_DIR="all_controllers/"
TOP_DIR="top_controllers/"

for dir in ${ALL_DIR}*/; do

top_cont=$(ls ${dir} -1 | tail -n 1)
cp "${dir}${top_cont}" "${TOP_DIR}${top_cont}" -v

done
