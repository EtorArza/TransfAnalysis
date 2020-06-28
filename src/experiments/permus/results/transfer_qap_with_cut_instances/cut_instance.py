import sys


if len(sys.argv) != 4:
    print("Example: \n\npython input_instance.txt output_instance.txt n \n\n where n is the size of the output instance.")
    exit(1)

in_path = sys.argv[1]
out_path = sys.argv[2]
n = int(sys.argv[3])
N = -1

lines = []
with open(in_path,"r") as file:
    for line in file:
        if N == -1:
            N = int(line.strip("\n"))
            continue
        new_line = line.strip("\n").split(" ")
        new_line = [str(int(el)) for el in new_line if el.isdigit()]
        lines.append(new_line)

for n_out_instance in range(int(N / n)):
    with open(out_path+"_"+str(n_out_instance)+".qap","w") as file:
        print(n, file=file, end="\n")
        k = n*n_out_instance
        for i in range(n):
            print(" ".join(lines[i+k][k:k+n]), file=file, end="\n")
        for i in range(n):
            print(" ".join(lines[N+k+i][k:k+n]), file=file, end="")
            if i != n-1:
                print("\n", file=file, end="")
            



print(sys.argv)
