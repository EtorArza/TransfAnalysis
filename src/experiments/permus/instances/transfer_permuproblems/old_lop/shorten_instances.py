import sys

filename = sys.argv[1]
result_filename = sys.argv[2]

n = 75

with open(filename, "r") as f:
    with open(result_filename, "w") as f_out:
        content = f.readlines()[1:]
        f_out.writelines(str(n)+"\n")
        for line in content[0:n]:
            line = line.strip(" ")
            values = line.split(" ")[0:n]
            line_to_write = " "+" ".join(values) + "\n"
            f_out.write(line_to_write)
