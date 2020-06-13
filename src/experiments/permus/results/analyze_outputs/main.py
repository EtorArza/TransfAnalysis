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
from tqdm import tqdm as tqdm

#save_fig_path = "/home/paran/Dropbox/BCAM/02_NEAT_permus/paper/images/analyze_outputs/"
save_fig_path = "/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus/results/analyze_outputs/figures/"

#input_file = "/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus/results/analyze_outputs/responses_journal.txt"


DATASET_list = [
"PERMUPROB",
"PERMUPROB",
"QAP",
"QAP"
]

input_file_list = [
    "/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus/results/4by4_permu_problems/result_response_transfer_permuproblem_0_1s_2h.txt",
    "/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus/results/4by4_permu_problems/result_response_transfer_permuproblem_0_25s_1h.txt",
    "/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus/results/transfer_qap_with_cut_instances/result_response_transfer_qap_0_1s_2h.txt",
    "/home/paran/Dropbox/BCAM/NEAT_code/src/experiments/permus/results/transfer_qap_with_cut_instances/result_response_transfer_qap_0_25s_1h.txt"
]


for DATASET, input_file in zip(DATASET_list, input_file_list):

    print(input_file.split("/")[-1])

    if DATASET== "PERMUPROB":

        def get_class_of_instance(controller_and_instance):
            return (controller_and_instance[0].split(".")[-2].split("_")[0], controller_and_instance[1].split(".")[-1])
        def clean_name(ins_name):
            if "controller" in ins_name:
                return ins_name.split("/")[-1].split("_best")[0]
            elif "instances" in ins_name:
                return ins_name.split("/")[-1]

        def get_type_of_instance(clean_instance_name):
            return clean_instance_name.split(".")[-1]


    elif DATASET=="QAP":
        def get_class_of_instance(controller_and_instance):
            return (controller_and_instance[0].split("/")[-1][0], controller_and_instance[1].split("/")[-1][0])
        def clean_name(ins_name):
            if "controller" in ins_name:
                return ins_name.split("/")[-1].split("_best")[0]
            elif "instances" in ins_name:
                return ins_name.split("/")[-1]

        def get_type_of_instance(clean_instance_name):
            return clean_instance_name[0]






    class instance_class_connector:


        def __init__(self, line_list):
            self._classes_dict = dict()
            self.line_list = line_list[:]
            for item in line_list:
                self.add_item((item[0], item[1]), item[2])

        def add_item(self, instance: Tuple[str] , response: List[float]):
            class_of_instance = get_class_of_instance(instance)
            if class_of_instance not in self._classes_dict:
                self._classes_dict[class_of_instance] = [list(), list()]
                self.add_item(instance, response)
            else:
                self._classes_dict[class_of_instance][0].append(   tuple( clean_name(el) for el in instance )   )
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

        def get_all_train_instances(self):
            # self._classes_dict -> 
            #   (train_class,test_class): [  [(train_ins0, test_ins0), (train_ins1, test_ins1), (train_ins2, test_ins2)]    ,     [response_0 , response_1, response_2]  ]
            all_classes = self.get_all_classes() 
            instances = set()
            for class_type in all_classes:
                instances.update(      [i for i,j in self._classes_dict[class_type][0]]      )

            return sorted(list(instances))

        def get_all_test_instances(self):
            # self._classes_dict -> 
            #   (train_class,test_class): [  [(train_ins0, test_ins0), (train_ins1, test_ins1), (train_ins2, test_ins2)]    ,     [response_0 , response_1, response_2]  ]
            all_classes = self.get_all_classes() 
            instances = set()
            for class_type in all_classes:
                instances.update(      [j for i,j in self._classes_dict[class_type][0]]      )

            return sorted(list(instances))


        def get_all_classes(self):
            return sorted([*self._classes_dict.keys()])

        def average_response_of_class(self, class_of_instances):
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

        def avg_distance_between_responses(self, list_of_responses):
            dist = 0
            num = 0
            visited = set()
            for i, resp_0 in enumerate(list_of_responses): 
                for j, resp_1 in enumerate(list_of_responses):
                    if i==j:
                        continue
                    if (i,j) in visited or (j,i) in visited:
                        continue
                    else:
                        visited.add((i,j))
                    dist += float(abs(pd.DataFrame(resp_0) - pd.DataFrame(resp_1)).sum().sum())
                    num += 1
            return dist / num


        def get_avge_avge_L1_distance_to_mean_response_df(self):
            index = sorted(list(set([key[0] for key in self._classes_dict]))) + ["all"]
            columns = sorted(list(set([key[1] for key in self._classes_dict]))) + ["all"]
            res = pd.DataFrame(data=np.zeros((len(index), len(columns))), index = index, columns = columns)
            res.loc["all","all"] = np.nan
            for ind in index:
                for col in columns:
                    res.loc[ind, col] = self.avge_L1_distance_to_mean_response(self.responses_given_class((ind,col)))
            return res





        def return_dataframe_with_responses_on_train_instances(self):
            index = sorted(list(set([key[0] for key in self._classes_dict])))
            columns = sorted(list(set([key[1] for key in self._classes_dict])))


            

            all_train_instances = self.get_all_train_instances()
            all_test_instances = self.get_all_test_instances()


            list_of_all_train_instances = [ clean_name(line[0]) for line in self.line_list ]
            list_of_all_test_instances  = [ clean_name(line[1]) for line in self.line_list ]
            list_of_all_responses       = [            line[2]  for line in self.line_list ]



            print("Avg L1 distance between two responses trained in same instance:", end = " ")
            fix_training_dist = []
            for training_instance in sorted(list(set(list_of_all_train_instances))):
                corresponding_responses = [list_of_all_responses[i]  for i in range(len(list_of_all_train_instances)) if list_of_all_train_instances[i] == training_instance]
                fix_training_dist.append(self.avg_distance_between_responses(corresponding_responses))
            print(mean(fix_training_dist))

            fix_test_dist = []
            print("Avg L1 distance between two responses tested in the same instance:", end = " ")
            for test_instance in sorted(list(set(list_of_all_test_instances))):
                corresponding_responses = [list_of_all_responses[i]  for i in range(len(list_of_all_test_instances)) if list_of_all_test_instances[i] == test_instance]
                fix_test_dist.append(self.avg_distance_between_responses(corresponding_responses))
            print(mean(fix_test_dist))

            avged_responses = []
            unique_training_instances = sorted(list(set(list_of_all_train_instances)))

            for training_instance in unique_training_instances:
                corresponding_responses = [list_of_all_responses[i]  for i in range(len(list_of_all_train_instances)) if list_of_all_train_instances[i] == training_instance]
                avg_response_to_be_appended = list(pd.DataFrame(corresponding_responses).mean())
                avged_responses.append(avg_response_to_be_appended)

            return pd.DataFrame({"response":avged_responses, "train_instance":unique_training_instances})








            

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

    def save_fig(d, fig_title, fig_path, scale_between_1_and_0 = False):

        data = d.copy(deep=True)

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



    def plot_MDS_avgd_on_train(responses_obj, file_name):
        avgd_responses = responses_obj.return_dataframe_with_responses_on_train_instances()
        trained_classes = list(set([el[0] for el in responses_obj.return_list_of_corresponding_classes()]))

        df = pd.DataFrame(MDS_2d(avgd_responses["response"]))



        colors = list(matplotlib.colors.TABLEAU_COLORS)

        


        for idx, train_instance in enumerate(avgd_responses["train_instance"]):
            xi = np.array(df.iloc[idx,0])
            yi = np.array(df.iloc[idx,1])
            color_idx = trained_classes.index(get_type_of_instance(train_instance))
            ci = colors[color_idx] #color for ith feature 


            
            label = None

            plt.scatter(xi,yi,marker=",", color=ci, label=label) 

        for idx, t_class in enumerate(trained_classes):
            label = str("A = " + t_class)
            plt.scatter([],[], marker=",", color=colors[idx], label=label) 



        plt.legend(fontsize=8, markerscale=0.5)
        plt.tight_layout()
        plt.savefig(file_name)
        plt.close()


    # icc { 
    #   (train_class,test_class): [    [(train_ins0, test_ins0), (train_ins1, test_ins1), (train_ins2, test_ins2)]    ,     [response_0 , response_1, response_2]    ],  
    #   (train_class,test_class): [    [(train_ins0, test_ins0), (train_ins1, test_ins1), (train_ins2, test_ins2)]    ,     [response_0 , response_1, response_2]    ]...
    # } 


    line_list_read = []
    with open(input_file, "r") as f:
        for line in f:
            line_list_read.append(eval(line.strip("\n")))


    icc = instance_class_connector(line_list_read)


    filename_from_input_file_path = input_file.split("/")[-1].split(".")[0]

    plot_MDS_avgd_on_train(icc, save_fig_path+"MDS_avged_on_train"+filename_from_input_file_path+".pdf")
    save_fig(icc.get_avge_avge_L1_distance_to_mean_response_df(), "", save_fig_path+filename_from_input_file_path+".pdf")
    plot_MDS(icc, save_fig_path+"MDS"+filename_from_input_file_path+".pdf")
