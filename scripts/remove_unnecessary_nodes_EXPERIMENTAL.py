
if __name__ == "__main__":
    print("Introduce genome result: ")
    print("")


    lines = []
    while True:
        line = input()
        if line:
            lines.append(line.strip(" "))
        else:
            break



def remove_not_necesary(lines):




    lines_copy = lines[:]


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



    connected_nodes = set()

    for _ in range(10):
        output_nodes = []
        lines = lines_copy[:]
        line = lines.pop(0)
        while "node" not in line:
            line = lines.pop(0)

        n_input = len(list(filter(lambda x: "node" in x and x[-1]=="1", lines)))
        n_output = len(list(filter(lambda x: "node" in x and x[-1]=="2", lines)))
        n_hidden = len(list(filter(lambda x: "node" in x and x[-1]=="3", lines)))

        counter = names()
        n_flag = 1
        while "node" in line:
            splt_line = line.split(" ")




            if splt_line[3] == "2":
                output_nodes.append(splt_line[1])
                connected_nodes.add(splt_line[1])

            
            
            
            line = lines.pop(0)




        counter = names()
        n_flag = 0
        while "gene" in line:
            splt_line = line.split(" ")
            if splt_line[2] == "1":
                line = lines.pop(0)
                continue
            
            node_from = splt_line[2]
            node_to = splt_line[3]

            if node_to in output_nodes or node_to in connected_nodes:
                connected_nodes.add(node_from)


            line = lines.pop(0)


    # \path [->] (A) edge node {$5$} (B);
    # \path [->] (B) edge node {$3$} (C);
    # \path [->] (A) edge node {$4$} (D);
    # \path [->] (D) edge node {$3$} (C);
    # \path [->] (A) edge node {$3$} (E);
    # \path [->] (D) edge node {$3$} (E);
    # \path [->] (D) edge node {$3$} (F);
    # \path [->] (C) edge node {$5$} (F);
    # \path [->] (E) edge node {$8$} (F); 
    # \path [->] (B) edge[bend right=60] node {$1$} (E); 
    # \end{scope}
    # \end{tikzpicture}

    rel_lines = []

    for line in lines_copy:
        if "node" in line and line.split(" ")[1] not in connected_nodes:
            continue
        elif "gene" in line and (line.split(" ")[2] not in connected_nodes or line.split(" ")[3] not in connected_nodes):
            continue
        else:
            rel_lines.append(line)
    
    return rel_lines