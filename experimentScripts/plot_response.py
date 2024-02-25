from statistics import mean, variance, stdev, median
from joblib import PrintTime
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors
import subprocess
import re
from tqdm import tqdm as tqdm
import sklearn
from sklearn.neighbors import KernelDensity
from sklearn.discriminant_analysis import LinearDiscriminantAnalysis as LDA
from scipy import stats
from sklearn import preprocessing
from sklearn import manifold
from sklearn.preprocessing import StandardScaler

# # Load scores from transferability.
# import plot_transferability







txt_paths = [
"experimentResults/transfer_permus_tsp/results/response.txt",
"experimentResults/transfer_permus_qap/results/response.txt",
"experimentResults/transfer_permus_problems/results/response.txt",
"experimentResults/transfer_16_continuous_problems/results/response.txt",
"experimentResults/transfer_generated_continuous/results/response.txt",
]


transfer_exp_list =[
"TSP",
"QAP",
"PERMUPROB",
"continuous12",
"rokkonen"
]


def reduce_dimensionality_to_2d(response_dict_fit, response_dict_predict, method):


    response_array_fit = np.vstack([v for k, v in response_dict_fit.items()])
    response_array_predict = np.vstack([v for k, v in response_dict_predict.items()])

    target_class = np.asarray([el.split("|")[0] for el in response_dict_fit.keys()])

    # # MDS
    # m =  response_array_predict.shape[0]
    # dist_matrix_dict = np.zeros((m,m))
    # for i in range(m):
    #     for j in range(m):
    #         # dist_matrix_dict[i,j] = np.log(1 + hamming_dist(pop[i], pop[j])) / np.log(1+ n)
    #         dist_matrix_dict[i,j] = L1_distance_between_responses(response_array_predict[i], response_array_predict[j])
    # embedding = sklearn.manifold.MDS(dissimilarity="precomputed", n_components=2, n_init=3, max_iter=500, n_jobs=8)
    # res = np.array(embedding.fit_transform(dist_matrix_dict))

    # # TSNE 
    # # https://scikit-learn.org/stable/modules/generated/sklearn.manifold.TSNE.html
    # # https://en.wikipedia.org/wiki/T-distributed_stochastic_neighbor_embedding
    # tsne = sklearn.manifold.TSNE(n_components=2, perplexity=5, learning_rate='auto', metric="l1")
    # res = tsne.fit_transform(response_array_predict)

    if method == "PCA":
        pca = sklearn.decomposition.PCA(n_components=2)
        embedding = pca.fit(response_array_fit)
        res = embedding.transform(response_array_predict)

    elif method == "LDA":
        # https://towardsdatascience.com/dimensionality-reduction-approaches-8547c4c44334
        lda = LDA(n_components=2)
        embedding = lda.fit(response_array_fit, target_class)
        res = embedding.transform(response_array_predict)


    else:
        raise ValueError(f"Method {method} not recognized.")
    
    res = pd.DataFrame(res, index=[el for el in response_dict_predict.keys()])
    return res


def get_2D_embedding_from_features(input_csv):
    
    df = pd.read_csv(input_csv, header=None, index_col=0)
    scaler = StandardScaler()
    df_standardized = scaler.fit_transform(df)

    pca = sklearn.decomposition.PCA(n_components=2, )
    embedding = pca.fit(df_standardized)
    res = embedding.transform(df_standardized)
    res = pd.DataFrame(res, index=[el for el in df.index])
    return res

def get_2D_embedding_from_response(input_txt, method):

    assert method in ["PCA", "LDA"]

    def remove_seed_from_instance_name(instance_name):
        if 'seed' not in instance_name:
            print("ERROR: Controller name should include _seedk_best.controller, where k is the seed used to train the controller.")
            exit(1)
        instance_name = re.sub(r"seed\d*_", '', instance_name)
        return instance_name

    def sort_key(x):
        if x.isnumeric():
            return float(x)
        elif x.strip("_ ").isnumeric():
            return float(x.strip("_ "))
        else:
            res = ""
            res += x.split(".")[-1]
            res += x
            return res

    def get_train_instances(data_frame):
        return sorted(list(data_frame["train_name"].unique()), key=sort_key)

    def get_test_instances(data_frame):
        return sorted(list(data_frame["test_name"].unique()), key=sort_key)

    def get_row_index(data_frame, train_name, test_name):
        element = data_frame[(data_frame["train_name"] == train_name) & (data_frame["test_name"] == test_name)]
        if len(element)==0:
            return None
        else:
            return element.index[0]

    def nicefyInstanceName(x: str):
        if transfer_exp == "continuous12":
            return "$A_{"+x.strip("_") + "}$"
        elif transfer_exp == "QAP":
            return x.split("_")[0]
        else:
            return x.strip("_")

    def L1_distance_between_responses(resp1, resp2):
        return sum(abs(resp1 - resp2))


    responses = []

    data_frame = pd.DataFrame(columns=["response", "train_name", "test_name", "train_type", "test_type", "train_seed", "resp_diff_to_average_same_train", "resp_diff_to_global_average", "diff_higher_to_global"])

    print(f"Reading file {input_txt}")
    with open(input_txt) as f:
        for line in f:
            train_seed = line.split("seed")[-1].split("_")[0]
            line = remove_seed_from_instance_name(line)
            # Trained only in one
            if transfer_exp == "continuous12":
                if not "Only" in line:
                    continue
                line = eval(line)
                train_name = "_"+str(line[0].split("TrainOnlyInF_")[1].split("_best.controller")[0])+"_"
                test_name = "_"+str(line[1])+"_"
                response = np.array(line[-1])

            elif transfer_exp == "rokkonen":
                line = eval(line)
                train_name = line[0].split("NLO_")[-1].split("_best.controlle")[0]
                test_name = str(line[-1])
                response = np.array(line[-2])


            # PERMUS (both qap and permuproblems)
            elif transfer_exp == "QAP_MULTI" or transfer_exp == "QAP" or transfer_exp=="PERMUPROB" or transfer_exp=="TSP":
                line = eval(line)
                train_name = line[0].split("/")[-1].split("_best.controlle")[-2]
                test_name = line[1].split("/")[-1]
                # # Dont test the controller in an instance, when that instance was used to train it.
                # if train_name in test_name:
                #     continue
                response = np.array(line[2])

            else:
                raise ValueError(f"transfer_exp = {transfer_exp} not recognized!")


            # print("train_type",train_type, "test_type", test_type)

            # score_df = plot_transferability.every_data_frame_list[idx]
            # score = score_df[(score_df["train_name"] == train_name) & (score_df["test_name"] == test_name) & (score_df["train_seed"] == train_seed)]["score"]

            resp_diff_to_average_same_train, resp_diff_to_global_average, diff_higher_to_global = None, None, None

            new_row_df = pd.DataFrame([[response, train_name, test_name, train_seed, resp_diff_to_average_same_train, resp_diff_to_global_average, diff_higher_to_global]], 
                            columns=["response", "train_name", "test_name", "train_seed", "resp_diff_to_average_same_train", "resp_diff_to_global_average", "diff_higher_to_global"])
            data_frame = pd.concat([data_frame, new_row_df], ignore_index=True)

        # compute average distance between responses in train/test

        all_seeds = list(data_frame["train_seed"].unique()) 
        data_frame.insert(1, "transferability", [-1]*len(data_frame.index), False)

        distance_between_same_train_list_diff_seed = list()
        distance_between_same_train_list_same_seed = list()
        distance_between_both_diff_list = list()
        distance_between_same_test_list = list()



        train_names = sorted(data_frame["train_name"].unique(), key=sort_key)
        test_names = sorted(data_frame["test_name"].unique(), key=sort_key)

        N_distances_AVERAGED_per_instance = 5
        np.random.seed(3)

    # Compute avg response L1 distance between same train instances & different train seed.
    for train_name in tqdm(train_names):
        for reps in range(N_distances_AVERAGED_per_instance):
            seed1 = np.random.choice(all_seeds)
            seed2 = np.random.choice([el for el in all_seeds if el != seed1])
            r1_list = data_frame[(data_frame["train_name"] == train_name) & (data_frame["train_seed"] == seed1)]["response"]
            r2_list = data_frame[(data_frame["train_name"] == train_name) & (data_frame["train_seed"] == seed2)]["response"]
            r1 = np.random.choice(r1_list)
            r2 = np.random.choice(r2_list)
            distance_between_same_train_list_diff_seed.append(L1_distance_between_responses(r1,r2))
    distance_between_same_train_diff_seed = np.average(distance_between_same_train_list_diff_seed)


    # Compute avg response L1 distance between same test instances
    for test_name in tqdm(test_names):
        for reps in range(N_distances_AVERAGED_per_instance):
            seed1 = np.random.choice(all_seeds)
            seed2 = np.random.choice([el for el in all_seeds if el != seed1])
            r1_list = data_frame[(data_frame["test_name"] == test_name) & (data_frame["train_seed"] == seed1)]["response"]
            r2_list = data_frame[(data_frame["test_name"] == test_name) & (data_frame["train_seed"] == seed2)]["response"]
            r1 = np.random.choice(r1_list)
            r2 = np.random.choice(r2_list)
            distance_between_same_test_list.append(L1_distance_between_responses(r1,r2))

    distance_between_same_test = np.average(distance_between_same_test_list)


    # Compute avg response L1 distance between same train instances & same train seed.
    for train_name in tqdm(train_names):
        for reps in range(N_distances_AVERAGED_per_instance):
            seed1 = np.random.choice(all_seeds)
            seed2 = seed1
            r1_list = data_frame[(data_frame["train_name"] == train_name) & (data_frame["train_seed"] == seed1)]["response"]
            r2_list = data_frame[(data_frame["train_name"] == train_name) & (data_frame["train_seed"] == seed2)]["response"]

            r1 = np.random.choice(r1_list)
            r2 = np.random.choice(r2_list)

            k = 0
            while(sum(r1) == sum(r2)):
                r2 = np.random.choice(r2_list)


            distance_between_same_train_list_same_seed.append(L1_distance_between_responses(r1,r2))
    distance_between_same_train_same_seed = np.average(distance_between_same_train_list_same_seed)

    # Compute avg response L1 distance between diff train instances & diff train seed.
    for train_name in tqdm(train_names):
        for reps in range(N_distances_AVERAGED_per_instance):
            seed1 = np.random.choice(all_seeds)
            seed2 = seed1
            test_name = np.random.choice([el for el in test_names if el != train_name])
            
            r1_list = data_frame[(data_frame["train_name"] == train_name) & (data_frame["test_name"] == test_name) & (data_frame["train_seed"] == seed1)]["response"]
            r2_list = data_frame[(data_frame["train_name"] != train_name) & (data_frame["test_name"] != test_name) & (data_frame["train_seed"] == seed2)]["response"]

            r1 = np.random.choice(r1_list)
            r2 = np.random.choice(r2_list)

            k = 0
            while(sum(r1) == sum(r2)):
                r2 = np.random.choice(r2_list)


            distance_between_both_diff_list.append(L1_distance_between_responses(r1,r2))
    distance_between_both_diff_same_seed = np.average(distance_between_both_diff_list)


    print("---------")
    print("Problem name: ", transfer_exp)
    print("Avg L1 distance among responses TRAINED IN SAME instance (diff train seed):", distance_between_same_train_diff_seed)
    print("Avg L1 distance among responses TRAINED IN SAME instance (same train seed):", distance_between_same_train_same_seed)
    print("Avg L1 distance among responses TESTED IN SAME instance:", distance_between_same_test)
    print("Avg L1 distance among responses TRAINED & TESTED IN DIFFERENT instances:", distance_between_both_diff_same_seed)
    print("---------")



    # Get responses averaged per train and seed
    response_dict_averaged_per_train_instance_and_seed = {}
    same_train_variance_list = []
    for train_name in tqdm(train_names):
        responses_in_same_train = []        
        for train_seed in data_frame["train_seed"].unique():
            average_response = np.apply_along_axis(mean, 0, np.asarray([el for el in data_frame[(data_frame["train_name"] == train_name) & (data_frame["train_seed"] == train_seed)]["response"]]))    
            response_dict_averaged_per_train_instance_and_seed[train_name+"|"+str(train_seed)] = average_response
            responses_in_same_train.append(average_response)
        same_train_variance_list.append(np.var(np.asarray(responses_in_same_train), axis=0))

    # Get responses averaged per train
    response_dict_averaged_per_train_instance = {}
    for train_name in train_names:
        sub_df_with_certain_train_instance = data_frame[(data_frame["train_name"] == train_name)]

        response_list = np.asarray([el for el in sub_df_with_certain_train_instance["response"]])
        median_response = np.apply_along_axis(median, 0, response_list)
        average_response = np.apply_along_axis(mean, 0, response_list)


        response_dict_averaged_per_train_instance[train_name] = average_response
    # response_list_averaged_per_train_instance = np.array([np.array(resp) for resp in response_list_averaged_per_train_instance])

    response_dict_fit = response_dict_averaged_per_train_instance_and_seed
    response_dict_predict = response_dict_averaged_per_train_instance
    
    return reduce_dimensionality_to_2d(response_dict_fit, response_dict_predict, method)


# # # For debugging python with interactive shell. Start interactive shell.
# import code
# code.interact(local=locals())



def plot_2D_embedding(df, transfer_exp, save_fig_path):


    # Get the type of each instance. Not used during the generation of the embedding.
    if transfer_exp == "QAP":
        def get_type(instance_name):
            if "taixxA" in instance_name:
                return "A"
            elif "taixxB" in instance_name:
                return "B"
            elif "sko" in instance_name:
                return "sko" 
            else:
                raise ValueError(f"instance_name format is incorrect, instance_name={instance_name}")

    elif transfer_exp == "PERMUPROB":
        def get_type(instance_name):
            return instance_name.split(".")[-1]
    elif transfer_exp == "QAP_MULTI":
        def get_type(instance_name):
            type_name = ""
            if "A" in instance_name:
                type_name += "A"
            if "B" in instance_name:
                type_name += "B"
            if "C" in instance_name:
                type_name += "C"
            if len(type_name)==2:
                type_name = type_name[0] + "_" + type_name[1]
            return type_name
    elif transfer_exp == "LOO16" or transfer_exp == "continuous12":
        PROBLEM_TYPES = ["bowl", "multiopt"]
        def get_type(instance_name):
            print("ERROR: type is wrong, and should instead be set based on experimental results in the paper.")
            exit(1)
            problem_index = int(instance_name.split("seed")[0].strip("_"))
            type_name = PROBLEM_TYPES[(problem_index - 1) // 6]
            return type_name
    elif transfer_exp == "rokkonen":
        def get_type(instance_name):
            return instance_name
    elif transfer_exp == "TSP":
        df.sort_index(axis=0, ascending=False, inplace=True, kind='quicksort', key= lambda x: pd.Index(map(lambda y: y.count("kro")*2+y.count("pr"), x)))
        def get_type(instance_name):
            if "kro" in instance_name:
                return "kro"
            elif "pr" in instance_name:
                return "pr"
            else:
                return "other"
    else:
        raise ValueError(f"Experiment {transfer_exp} not recognized.")
 


    colors = list(matplotlib.colors.TABLEAU_COLORS)
    markers = [".", "x", "^", "1"]

    
    fig, ax = plt.subplots(figsize=(3.5, 2.75))

    used_labels = set()
    for idx, train_instance in enumerate(df.index):
        xi = np.array(df.iloc[idx,0])
        yi = np.array(df.iloc[idx,1])
        train_instance = str(train_instance)
        if transfer_exp == "QAP":
            color_index = 0 if "A" in train_instance else 1 if "B" in train_instance else 2 if "sko" in train_instance else None
            assert color_index != None, f"train_instance = {train_instance}"  
            label = ["taixxA", "taixxB", "sko"][color_index]

        elif transfer_exp == "PERMUPROB":
            if ".tsp" in train_instance:
                color_index = 0
            elif ".lop" in train_instance:
                color_index = 1
            elif ".pfsp" in train_instance:
                color_index = 2
            elif ".qap" in train_instance:
                color_index = 3
            label = ["TSP", "LOP", "PFSP", "QAP"][color_index]

        elif transfer_exp == "continuous12":
            color_index = 1 if train_instance in ("_6_", "_11_", "_2_", "_1_", "_5_") else (2 if train_instance in ("_8_", "_4_", "_7_", "_3_") else (0 if train_instance in ("_9_", "_10_", "_12_") else None)) 
            label = "Type " + str(color_index+1)
        elif transfer_exp == "rokkonen":
            color_index = 0
            label = None
            ax.annotate(train_instance.removeprefix("NLO_"), (xi, yi))
        elif transfer_exp == "TSP":
            color_index = ["kro", "pr", "other"].index(get_type(train_instance))
            label = get_type(train_instance)
            # ax.annotate(train_instance, (xi, yi))
        else:
            raise ValueError(f"transfer_exp = {transfer_exp} not recognized.")
        ci = colors[color_index]
        ma = markers[color_index]
            
        if label not in used_labels:
            used_labels.add(label)
        else:        
            label = None

        ax.scatter(xi,yi,marker=ma, color=ci, label=label)

    legend_location_params = {"bbox_to_anchor":(0, 1.05, 1, 0.2), "loc":"lower left", "mode":"expand", "borderaxespad":0, "ncol":4}


    if transfer_exp == "continuous12":
        handles, labels = plt.gca().get_legend_handles_labels()
        order = [2,0,1]
        ax.legend([handles[idx] for idx in order],[labels[idx] for idx in order], fontsize=8, markerscale=1.0, **legend_location_params)
    if transfer_exp == "rokkonen":
        pass
    else:
        ax.legend(fontsize=8, markerscale=1.0, **legend_location_params)
    if "MDS" in save_fig_path:
        ax.set_xlim(-0.6,0.6)
        ax.set_ylim(-0.6,0.6)
    fig.tight_layout()
    fig.savefig(save_fig_path)
    plt.close()

# # MDS Transferability
# data = pd.read_csv("experimentResults/problem_analisys/transferability_MDS_continuous12.txt", index_col=0, header=0)
# plot_2D_embedding(data, "continuous12", "experimentResults/problem_analisys/figures/transferability_MDS_continuous12.pdf")

# data = pd.read_csv("experimentResults/problem_analisys/transferability_MDS_PERMUPROB.txt", index_col=0, header=0)
# plot_2D_embedding(data, "PERMUPROB", "experimentResults/problem_analisys/figures/transferability_MDS_PERMUPROB.pdf")

# data = pd.read_csv("experimentResults/problem_analisys/transferability_MDS_QAP.txt", index_col=0, header=0)
# plot_2D_embedding(data, "QAP", "experimentResults/problem_analisys/figures/transferability_MDS_QAP.pdf")

# data = pd.read_csv("experimentResults/problem_analisys/transferability_MDS_rokkonen.txt", index_col=0, header=0)
# plot_2D_embedding(data, "rokkonen", "experimentResults/problem_analisys/figures/transferability_MDS_rokkonen.pdf")

data = pd.read_csv("experimentResults/problem_analisys/transferability_MDS_TSP.txt", index_col=0, header=0)
plot_2D_embedding(data, "TSP", "experimentResults/problem_analisys/figures/transferability_MDS_TSP.pdf")


# # Matilda InstanceSpace
# data = pd.read_csv("other_src/InstanceSpace/trial_continuous12/coordinates.csv", index_col=0)
# plot_2D_embedding(data, "continuous12", "experimentResults/problem_analisys/figures/MatildaInstanceSpace_continuous12.pdf")

# data = pd.read_csv("other_src/InstanceSpace/trial_rokkonen/coordinates.csv", index_col=0)
# plot_2D_embedding(data, "rokkonen", "experimentResults/problem_analisys/figures/MatildaInstanceSpace_rokkonen.pdf")

data = pd.read_csv("other_src/InstanceSpace/trial_tsp/coordinates.csv", index_col=0)
plot_2D_embedding(data, "TSP", "experimentResults/problem_analisys/figures/MatildaInstanceSpace_tsp.pdf")


# # PCA of featureMatrix
# data = get_2D_embedding_from_features("experimentResults/problem_analisys/featureMatrix_continuous12_ELA.txt")
# plot_2D_embedding(data, "continuous12", "experimentResults/problem_analisys/figures/featureMatrix_continuous12_ELA_PCA.pdf")

# data = get_2D_embedding_from_features("experimentResults/problem_analisys/featureMatrix_rokkonen_ELA.txt")
# plot_2D_embedding(data, "rokkonen", "experimentResults/problem_analisys/figures/featureMatrix_rokkonen_ELA_PCA.pdf")

data = get_2D_embedding_from_features("experimentResults/problem_analisys/featureMatrix_TSP_Matilda.txt")
plot_2D_embedding(data, "TSP", "experimentResults/problem_analisys/figures/featureMatrix_TSP_Matilda_PCA.pdf")

# Response of HH framework (ours)
for idx, input_txt, transfer_exp in zip(range(len(transfer_exp_list)), txt_paths, transfer_exp_list):
    if transfer_exp != "TSP":
        continue
    data = get_2D_embedding_from_response(input_txt, "LDA")
    plot_2D_embedding(data, transfer_exp, "experimentResults/problem_analisys/figures/response_LDA_"+transfer_exp+".pdf")
    data = get_2D_embedding_from_response(input_txt, "PCA")
    plot_2D_embedding(data, transfer_exp, "experimentResults/problem_analisys/figures/response_PCA_"+transfer_exp+".pdf")

import subprocess
subprocess.call("rsync -zarv --delete --prune-empty-dirs --include '*/' --include '*.pdf' --exclude '*' experimentResults/ ../paper/experimentResults/", shell=True)