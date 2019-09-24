print("Introduce genome result: ")
print("")

#from remove_unnecessary_nodes import remove_not_necesary
import random


node_names = {
    
    "2":r"swap \\ optimum", # input
    "3":r"exchange \\ optimum",
    "4":r"insert \\ optimum",
    "5":r"relative \\ ranking",
    "6":r"progress",
    "7":r"distance",
    "8":r"sparsity",
    "9":r"random \\ number",
    
    "10":r"- ls,\\ + move", # output
    "11":r"operator \\ swap",
    "12":r"operator \\exchange",
    "13":r"operator \\ insert",
    "14":r"repeat \\ last",
    "15":r"personal \\ best",
    "16":r"best \\ known",
    "17":r"the one \\ above",
    "18":r"accept \\ worse",
    "19":r"tabu"

}


class names:

    def __init__(self):
        self.index = 0
    
    def next(self):
        res = str(self.index)
        self.index += 9/self.max_number
        return res
    def reset(self):
        self.index = 0
    
    def set_max_number(self, n):
        self.max_number = n


def load_biases(lines, biases):
    lines_copy = lines[:]
    lines_with_biases = list(filter(lambda x: "gene" in x and x.split(" ")[2]=="1", lines_copy))
    for line in lines_with_biases:
        biases[line.split(" ")[3]] = line.split(" ")[4]






lines = []
while True:
    line = input()
    if line:
        lines.append(line)
    else:
        break

biases = dict()
load_biases(lines, biases)




print(r"\begin{tikzpicture}")
print(r"\begin{scope}[every node/.style={thick,draw}]")



line = lines.pop(0)
while "node" not in line:
    line = lines.pop(0)

n_input = len(list(filter(lambda x: "node" in x and x[-1]=="1", lines)))
n_output = len(list(filter(lambda x: "node" in x and x[-1]=="2", lines)))
n_hidden = len(list(filter(lambda x: "node" in x and x[-1]=="3", lines)))

counter = names()
n_flag = 1
output_nodes = set()
while "node" in line:
    splt_line = line.split(" ")
    name = splt_line[1]
    bias = 0 if name not in biases else biases[name]

    shape = ""

    if float(bias) > 0:
        shape = " [red, line width=" + str(float(biases[name])/10) + "mm] "
    elif float(bias) < 0:
        shape = " [blue, line width=" + str(-float(biases[name])/10) + "mm] "

    if splt_line[3] == "0": # bias node
        if n_flag == 0:
            n_flag +=1
        line = lines.pop(0)
        continue
    if splt_line[3] == "1": # input nodes 
        if n_flag == 1:
            counter.reset()
            counter.set_max_number(n_input)
            n_flag +=1
        pass
        print(r"\node"+ shape+ "("+name+") at (" + counter.next() + r",7) {\tiny \tabcolsep=0cm \begin{tabular}{c} "+node_names[name]+r" \end{tabular}};")
    elif splt_line[3] == "2":  # output nodes
        if n_flag == 2:
            counter.reset()
            counter.set_max_number(n_output)
            n_flag +=1
        pass
        name = splt_line[1]
        output_nodes.add(name)
        print(r"\node"+ shape+ "("+name+") at (" + counter.next() + r",0) {\tiny \tabcolsep=0cm \begin{tabular}{c} "+node_names[name]+r" \end{tabular}};")
    elif splt_line[3] == "3":  # hidden nodes
        if n_flag == 3:
            counter.reset()
            counter.set_max_number(n_hidden)
            n_flag +=1
        pass
        name = splt_line[1]
        print(r"\node"+ shape+ "("+name+") at (" + counter.next() + ","+str(3 + 2*random.random())+") {"+" };")
    
    
    
    line = lines.pop(0)



print(
r"""\end{scope}
\begin{scope}[>={Stealth[black]},
every node/.style={fill=none,circle}]"""
)


counter = names()
n_flag = 0
while "gene" in line:
    splt_line = line.split(" ")
    if splt_line[2] == "1":
        line = lines.pop(0)
        continue
    

    w = float("{0:.2f}".format(round(float(splt_line[4]),2)))/10
    c = ["blue","red"][int(float(w) > 0)]
    w = str(abs(w))

    if splt_line[2] == splt_line[3]: # if loop
        print(r"\path[->,, line width = "+w+"mm, "+c+",every loop/.style={looseness=5}] ("+splt_line[2] +r") edge  [in=120,out=60,loop] node { } ();")
    
    elif splt_line[2] in output_nodes and splt_line[3] in output_nodes: #if output nodes connected
        print(r"\path [->, line width = "+w+"mm, "+c+",looseness="+str(4/(abs(int(splt_line[2])- int(splt_line[3]))))+"] ("+splt_line[2]+") edge node {$ $} ("+splt_line[3]+");")
    else:
        print(r"\path [->, line width = "+w+"mm, "+c+"] ("+splt_line[2]+") edge node {$ $} ("+splt_line[3]+");")
    
    
    
    line = lines.pop(0)
print(
r"""\end{scope}
\end{tikzpicture}
"""
)

