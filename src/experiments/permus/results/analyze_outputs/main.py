import pandas as pd
import numpy as np
from sklearn.cluster import KMeans
import sklearn
from scipy.cluster import hierarchy
from scipy.cluster.hierarchy import dendrogram, linkage
from scipy.cluster.hierarchy import fcluster
import numpy as np
import matplotlib
#matplotlib.rcParams['pdf.fonttype'] = 42
#matplotlib.rcParams['ps.fonttype'] = 42
from matplotlib import pyplot as plt
from statistics import mean
from typing import Tuple, List, Callable

#save_fig_path = "/home/paran/Dropbox/BCAM/02_NEAT_permus/paper/images/analyze_outputs/"
save_fig_path = "/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus/results/analyze_outputs/figures/"

input_file = "/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus/results/analyze_outputs/responses_journal.txt"


line_list = []
with open(input_file, "r") as f:
    for line in f:
        line_list.append(eval(line.strip("\n")))


class instance_class_connector:

    _get_class_of_instance = None

    def __init__(self, function_from_instance_to_class):
        self._classes_dict = dict()
        self._get_class_of_instance = function_from_instance_to_class

    def add_item(self, instance: Tuple[str] , response: List[float]):
        class_of_instance = self.class_of_instance(instance)
        if class_of_instance not in self._classes_dict:
            self._classes_dict[class_of_instance] = [list(), list()]
            self.add_item(instance, response)
        else:
            self._classes_dict[class_of_instance][0].append(instance)
            self._classes_dict[class_of_instance][1].append(response)

    def responses_given_class(self, class_of_instances):
        if class_of_instances[0] == "all" and class_of_instances[1] == "all":
            return None

        if class_of_instances[0] == "all":
            res = []
            list_of_keys = list(set([key[1] for key in self._classes_dict]))
            for key in list_of_keys:
                res += self.responses_given_class((key, class_of_instances[1]))
            return res
        elif class_of_instances[1] == "all":
            res = []
            list_of_keys = list(set([key[0] for key in self._classes_dict]))
            for key in list_of_keys:
                res += self.responses_given_class((class_of_instances[0], key))
            return res
        
        else:
            return self._classes_dict[class_of_instances][1]

    def instances_of_a_certain_class(self, class_of_instances): 
        return self._classes_dict[class_of_instances][0]

    def class_of_instance(self, instance):
        return self._get_class_of_instance(instance)

    def get_all_classes(self):
        return list(self._classes_dict.keys)

    def average_response_of_class(self, class_of_instances):
        print(self.responses_given_class(class_of_instances))
        print(mean(self.responses_given_class(class_of_instances)))
        exit(1)
        return mean(self.responses_given_class(class_of_instances))
    
    def clear_items(self):
        self._classes_dict = dict()
    
    def response_of_instance(self, instance):
        key = (self.class_of_instance(instance[i]) for i in (0,1))
        for instance_from_dict, response in zip(self._classes_dict[key][0], self._classes_dict[key][1]):
            if instance == instance_from_dict:
                return response
    
    def subset(self, function_to_select_instances: Callable[[Tuple],bool], function_from_instance_to_class: Callable[[Tuple],Tuple]):
        result_subset = instance_class_connector(function_from_instance_to_class)
        for key in self._classes_dict:
            for instance, response in zip(self._classes_dict[key][0], self._classes_dict[key][1]):
                if function_to_select_instances(instance):
                    result_subset.add_item(instance, response)
        return result_subset
    
    def __repr__(self):
        return str(list([key for key in self._classes_dict]))
    

    def avge_L1_distance_to_mean_response(self, list_of_responses):
        tmp_df = pd.DataFrame(list_of_responses)
        return abs(tmp_df - tmp_df.mean()).mean().mean()

    def get_avge_avge_L1_distance_to_mean_response_df(self):
        index = sorted(list(set([key[0] for key in self._classes_dict]))) + ["all"]
        columns = sorted(list(set([key[1] for key in self._classes_dict]))) + ["all"]
        print(index)
        print(columns)
        res = pd.DataFrame(data=np.zeros((len(index), len(columns))), index = index, columns = columns)
        res.loc["all","all"] = np.nan
        for ind in index:
            for col in columns:
                print(ind, col)
                res.loc[ind, col] = self.avge_L1_distance_to_mean_response(self.responses_given_class((ind,col)))
        return res

    def return_all_responses(self):
        res = []
        for key in self._classes_dict:
            res += self._classes_dict[key][1] 
        return res

    def return_list_of_corresponding_classes(self):
        res = []
        for key in self._classes_dict:
            res += [key for _ in self._classes_dict[key][0]]
        return res




def get_class_of_instance_4by4(controller_and_instance):
    instance_names_4by4 = {
    'tai20_20_0': 'pfsp',
    'ch130': 'tsp',
    'kroA100': 'tsp',
    'tai50_5_0': 'pfsp',
    'pr136': 'tsp',
    'tai100b': 'qap',
    'tai20_20_1': 'pfsp',
    'tai80a': 'qap',
    'sko90': 'qap',
    'rat99': 'tsp',
    'tai100a': 'qap',
    'tai50_5_1': 'pfsp',
    'N-stabu3_150': 'lop',
    'N-t65d11xx_150': 'lop',
    'N-t70f11xx_150': 'lop',
    'N-be75np_150': 'lop'
    }
    ins_1_class = None
    ins_2_class = None
    for key in instance_names_4by4:
        if key in controller_and_instance[0]:
            ins_1_class = instance_names_4by4[key]
        if key in controller_and_instance[1]:
            ins_2_class = instance_names_4by4[key]
    if ins_1_class == None or ins_2_class == None:
        raise ValueError("ins_1_class or ins_2_class == None, (ins_1_class, ins_2_class) ==", (ins_1_class, ins_2_class))
    
    return (ins_1_class, ins_2_class)

def get_class_of_instance_cut_qap(controller_and_instance, size_matters, type_matters):
    
    res = ["", ""]
    for idx, instance in enumerate(controller_and_instance):
        if size_matters:
            if "cut30" in instance:
                res[idx] += "30"
            elif "cut60" in instance:
                res[idx] += "60"
            else:
                raise ValueError("Cut30 or cut60 not found in instance:", instance)
        if type_matters:
            if "0a" in instance:
                res[idx] += "Taixxa"
            elif "0b" in instance:
                res[idx] += "Taixxb"
            elif "sko" in instance:
                res[idx] += "Sko"
            else:
                raise ValueError("0a, 0b or Sko not found in instance:", instance)
    return (res[0], res[1])



permuprob_4by4 = instance_class_connector(get_class_of_instance_4by4)
cut_qap = instance_class_connector(lambda x: get_class_of_instance_cut_qap(x, True, True))

cut_qap_linelist = []
for item in line_list:
    if "4by4" in item[0]:
        permuprob_4by4.add_item((item[0], item[1]), item[2])
    else:
        cut_qap.add_item((item[0], item[1]), item[2])

cut_qap_small = cut_qap.subset(
    lambda x: "30" in get_class_of_instance_cut_qap(x,True , True)[0] and "30" in get_class_of_instance_cut_qap(x,True , True)[1], 
    lambda x: get_class_of_instance_cut_qap(x,False , True))

cut_qap_large = cut_qap.subset(
    lambda x: "60" in get_class_of_instance_cut_qap(x,True , True)[0] and "60" in get_class_of_instance_cut_qap(x,True , True)[1], 
    lambda x: get_class_of_instance_cut_qap(x,False , True))

cut_qap_taixxa = cut_qap.subset(
    lambda x: "Taixxa" in get_class_of_instance_cut_qap(x,True , True)[0] and "Taixxa" in get_class_of_instance_cut_qap(x,True , True)[1], 
    lambda x: get_class_of_instance_cut_qap(x,True , False))

cut_qap_taixxb = cut_qap.subset(
    lambda x: "Taixxb" in get_class_of_instance_cut_qap(x,True , True)[0] and "Taixxb" in get_class_of_instance_cut_qap(x,True , True)[1], 
    lambda x: get_class_of_instance_cut_qap(x,True , False))

cut_qap_sko = cut_qap.subset(
    lambda x: "Sko" in get_class_of_instance_cut_qap(x,True , True)[0] and "Sko" in get_class_of_instance_cut_qap(x,True , True)[1], 
    lambda x: get_class_of_instance_cut_qap(x,True , False))

list_of_response_objects = [permuprob_4by4, cut_qap_small, cut_qap_large, cut_qap_taixxa, cut_qap_taixxb, cut_qap_sko]
list_of_filenames = ["permuprob_4by4", "cut_qap_small", "cut_qap_large", "cut_qap_taixxa", "cut_qap_taixxb", "cut_qap_sko"]

#print(permuprob_4by4)
print(cut_qap_large.get_avge_avge_L1_distance_to_mean_response_df())

print("----------------")
print(cut_qap_taixxa)



def save_fig(d, fig_title, fig_path, scale_between_1_and_0 = False):

    data = d.copy(deep=True)
    print(data)

    if scale_between_1_and_0:
        plt.pcolor(data, vmin=0, vmax=1)
    else:
        plt.pcolor(data)


    test_label = r"test instances"
    train_label = r"train instances"

    FONTSIZE = 15
    plt.yticks(np.arange(0.5, len(data.index), 1), data.index, fontsize=FONTSIZE)
    plt.xticks(np.arange(0.5, len(data.columns), 1), data.columns, rotation = 90,  fontsize=FONTSIZE)
    plt.ylabel(train_label, fontsize=FONTSIZE*1.2)
    plt.xlabel(test_label, fontsize=FONTSIZE*1.2)
    plt.colorbar()
    plt.title(" ")
    #plt.title(fig_title)
    plt.tight_layout()
    plt.savefig(fig_path)
    plt.close()


def L1_dist(v1, v2):
    return sum(abs(np.array(v1) - np.array(v2)))

def MDS_2d(list_of_responses):

    m =  len(list_of_responses)
    n =  len(list_of_responses[0])
    dist_matrix_dict = np.zeros((m,m))
    
    for i in range(m):
        for j in range(m):
#            dist_matrix_dict[i,j] = np.log(1 + hamming_dist(pop[i], pop[j])) / np.log(1+ n)
            dist_matrix_dict[i,j] = L1_dist(list_of_responses[i], list_of_responses[j])

        
    embedding = sklearn.manifold.MDS(dissimilarity="precomputed", n_components=2, n_init=3, max_iter=500, n_jobs=8)
    res = np.array(embedding.fit_transform(dist_matrix_dict))

    return res



def plot_MDS(responses_obj, file_name):

    df = pd.DataFrame(MDS_2d(responses_obj.return_all_responses()))
    df = df.assign(trained_in=pd.Series([el[0] for el in responses_obj.return_list_of_corresponding_classes()]).values)
    df = df.assign(tested_on=pd.Series([el[1] for el in responses_obj.return_list_of_corresponding_classes()]).values)

    trained_in = list(set([el[0] for el in responses_obj.return_list_of_corresponding_classes()]))
    tested_on = list(set([el[1] for el in responses_obj.return_list_of_corresponding_classes()]))

    print(df)

    #generate a list of markers and another of colors 
    markers = ["." , "o" , "x" , "^" , "<", ">"]
    colors = list(matplotlib.colors.TABLEAU_COLORS)

    


    for i in range(len(trained_in)): #for each of the 7 features 
        for j in range(len(tested_on)): #for each of the 7 features 
            mi = markers[j] #marker for ith feature
            indexes = (df["trained_in"] == trained_in[i]) & (df["tested_on"] == tested_on[j])
            xi = np.array(df[indexes].iloc[:,0])
            yi = np.array(df[indexes].iloc[:,1])
            ci = colors[i] #color for ith feature 


            if j==0:
                label = str("A = " + trained_in[i])
                plt.scatter([],[], marker=",", color=ci, label=label) 
            if i==len(trained_in)-1:
                label = str("B = " + tested_on[j])
                plt.scatter([],[], marker=mi, color="black", label=label) 
            
            label = None

            plt.scatter(xi,yi,marker=mi, color=ci, label=label) 


    plt.legend(fontsize=8, markerscale=0.5)
    plt.tight_layout()
    plt.savefig(file_name)
    plt.close()



for name, df in zip(list_of_filenames, list_of_response_objects):
    save_fig(df.get_avge_avge_L1_distance_to_mean_response_df(), "", save_fig_path+name+".pdf")
    plot_MDS(df, save_fig_path+"MDS"+name+".pdf")

