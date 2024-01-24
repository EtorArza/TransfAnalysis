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

# # Load scores from transferability.
# import plot_transferability


save_fig_path = "experimentResults/"



save_fig_paths = [
"experimentResults/transfer_permus_qap/results/figures/",
"experimentResults/transfer_permus_problems/results/figures/",
"experimentResults/transfer_16_continuous_problems/results/figures/",
"experimentResults/transfer_generated_continuous/results/figures/",
]


txt_paths = [
"experimentResults/transfer_permus_qap/results/response.txt",
"experimentResults/transfer_permus_problems/results/response.txt",
"experimentResults/transfer_16_continuous_problems/results/response.txt",
"experimentResults/transfer_generated_continuous/results/response.txt",
]


transfer_exp_list =[
"QAP",
"PERMUPROB",
"Transfer16OnlyOne",
"transferGenerated"
]

for idx, input_txt, transfer_exp, save_fig_path in zip(range(len(transfer_exp_list)), txt_paths, transfer_exp_list, save_fig_paths):
    
    subprocess.run(f"mkdir -p {save_fig_path}", shell=True) # write out into log.txt

    def remove_seed_from_instance_name(instance_name):
        if 'seed' not in instance_name:
            print("ERROR: Controller name should include _seedk_best.controller, where k is the seed used to train the controller.")
            exit(1)
        instance_name = re.sub(r"seed\d*_", '', instance_name)
        return instance_name

    if transfer_exp == "QAP":
        def get_type(instance_name):
            return instance_name.split("_")[0][0]

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

    elif transfer_exp == "LOO16" or transfer_exp == "Transfer16OnlyOne":
        PROBLEM_TYPES = ["bowl", "multiopt"]
        def get_type(instance_name):
            problem_index = int(instance_name.split("seed")[0].strip("_"))
            type_name = PROBLEM_TYPES[(problem_index - 1) // 6]
            return type_name

    elif transfer_exp == "transferGenerated":
        def get_type(instance_name):
            return instance_name
            


    print(input_txt)








    responses = []

    data_frame = pd.DataFrame(columns=["response", "train_name", "test_name", "train_type", "test_type", "train_seed", "resp_diff_to_average_same_train", "resp_diff_to_global_average", "diff_higher_to_global"])

    print(f"Reading file {input_txt}")
    with open(input_txt) as f:
        for line in f:
            train_seed = line.split("seed")[-1].split("_")[0]
            line = remove_seed_from_instance_name(line)
            # Trained only in one
            if transfer_exp == "Transfer16OnlyOne":
                if not "Only" in line:
                    continue
                line = eval(line)
                train_name = "_"+str(line[0].split("TrainOnlyInF_")[1].split("_best.controller")[0])+"_"
                test_name = "_"+str(line[1])+"_"
                response = np.array(line[-1])

            elif transfer_exp == "transferGenerated":
                line = eval(line)
                train_name = line[0].split("NLO_")[-1].split("_best.controlle")[0]
                test_name = str(line[-1])
                response = np.array(line[-2])


            # PERMUS (both qap and permuproblems)
            elif transfer_exp == "QAP_MULTI" or transfer_exp == "QAP" or transfer_exp=="PERMUPROB":
                line = eval(line)
                train_name = line[0].split("/")[-1].split("_best.controlle")[-2]
                test_name = line[1].split("/")[-1]
                # # Dont test the controller in an instance, when that instance was used to train it.
                # if train_name in test_name:
                #     continue
                response = np.array(line[2])

            else:
                print(f"ERROR: transfer_exp = {transfer_exp} not recognized!")
                exit(1)

            train_type = get_type(train_name)
            test_type = get_type(test_name)

            # print("train_type",train_type, "test_type", test_type)

            # score_df = plot_transferability.every_data_frame_list[idx]
            # score = score_df[(score_df["train_name"] == train_name) & (score_df["test_name"] == test_name) & (score_df["train_seed"] == train_seed)]["score"]

            resp_diff_to_average_same_train, resp_diff_to_global_average, diff_higher_to_global = None, None, None

            new_row_df = pd.DataFrame([[response, train_name, test_name, train_type, test_type, train_seed, resp_diff_to_average_same_train, resp_diff_to_global_average, diff_higher_to_global]], 
                            columns=["response", "train_name", "test_name", "train_type", "test_type", "train_seed", "resp_diff_to_average_same_train", "resp_diff_to_global_average", "diff_higher_to_global"])
            data_frame = pd.concat([data_frame, new_row_df], ignore_index=True)

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
        if transfer_exp == "Transfer16OnlyOne":
            return "$A_{"+x.strip("_") + "}$"
        elif transfer_exp == "QAP":
            return x.split("_")[0]
        else:
            return x.strip("_")

    # compute average distance between responses in train/test

    all_seeds = list(data_frame["train_seed"].unique()) 
    data_frame.insert(1, "transferability", [-1]*len(data_frame.index), False)

    distance_between_same_train_list_diff_seed = list()
    distance_between_same_train_list_same_seed = list()
    distance_between_both_diff_list = list()
    distance_between_same_test_list = list()

    def L1_distance_between_responses(resp1, resp2):
        return sum(abs(resp1 - resp2))


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


    for response_index in range(data_frame.loc[0,"response"].shape[0]):

        # KDE for each response
        x_observed = np.array(list(data_frame["response"]))[:,response_index]
        x_observed = x_observed[np.abs(stats.zscore(x_observed)) < 5].reshape(-1,1)

        kde = KernelDensity(kernel='gaussian', bandwidth=0.05).fit(x_observed)

        x_plot_lim_lower = min(x_observed) - (max(x_observed) - min(x_observed)) / 10
        x_plot_lim_upper = max(x_observed) - (max(x_observed) + min(x_observed)) / 10

        x_plot_lim_lower = x_plot_lim_lower[0]
        x_plot_lim_upper = x_plot_lim_upper[0]

        x_plot = np.arange(x_plot_lim_lower,x_plot_lim_upper+ 0.00000001,0.0015).reshape(-1,1)
        y_plot = np.exp(kde.score_samples(x_plot))

        fig, ax = plt.subplots()
        ax.plot(x_plot, y_plot, label = "Density estimation")
        ax.plot(x_observed[:, 0], -0.005 - 0.01 * np.random.random(x_observed.shape[0]), "+k", label = "Observed datapoints")

        for train_name in train_names:        
            average_response = np.apply_along_axis(mean, 0, np.asarray([el for el in data_frame[(data_frame["train_name"] == train_name) & (data_frame["test_name"] == train_name)]["response"]]))
            ax.axvline(x=average_response[response_index],  color='r', linestyle='--', linewidth=0.5)
        ax.plot([], [], label='Average same train problem', color='r', linestyle='--', linewidth=0.5)


        global_avg_response = np.apply_along_axis(mean, 0, np.asarray([el for el in data_frame["response"]]))
        ax.axvline(x=global_avg_response[response_index], label="Average all train problems", color='orange', linestyle='-.', linewidth=0.5)
        ax.legend()



        fig.savefig(save_fig_path+"kde_reponse_indx_"+str(response_index)+".pdf")
        plt.close()


    # # For debugging python with interactive shell. Start interactive shell.
    # import code; code.interact(local=locals())
    # exit(1)

    # Get responses averaged per train and seed
    response_list_averaged_per_train_instance_and_seed = []
    target_class_list = []
    same_train_variance_list = []
    for train_name in tqdm(train_names):
        responses_in_same_train = []        
        for train_seed in data_frame["train_seed"].unique():
            average_response = np.apply_along_axis(mean, 0, np.asarray([el for el in data_frame[(data_frame["train_name"] == train_name) & (data_frame["train_seed"] == train_seed)]["response"]]))    
            response_list_averaged_per_train_instance_and_seed.append(average_response)
            responses_in_same_train.append(average_response)
            target_class_list.append(train_name)
        same_train_variance_list.append(np.var(np.asarray(responses_in_same_train), axis=0))

    # Get responses averaged per train
    response_list_averaged_per_train_instance = []
    for train_name in train_names:
        sub_df_with_certain_train_instance = data_frame[(data_frame["train_name"] == train_name)]

        response_list = np.asarray([el for el in sub_df_with_certain_train_instance["response"]])
        median_response = np.apply_along_axis(median, 0, response_list)
        average_response = np.apply_along_axis(mean, 0, response_list)


        response_list_averaged_per_train_instance.append(average_response)
    response_list_averaged_per_train_instance = np.array([np.array(resp) for resp in response_list_averaged_per_train_instance])
    
    def MDS_2d(response_array_fit, response_array_predict, target_class, method):
        
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
        return res





    def plot_MDS(response_array_fit, response_array_predict, target_class, file_name, method):


        df = pd.DataFrame(MDS_2d(response_array_fit, response_array_predict, target_class, method))

        colors = list(matplotlib.colors.TABLEAU_COLORS)
        markers = ["o", "x", "^", "1"]

        
        fig, ax = plt.subplots(figsize=(4.5, 3))

        used_labels = set()
        for idx, train_instance in enumerate(train_names):
            xi = np.array(df.iloc[idx,0])
            yi = np.array(df.iloc[idx,1])
            if transfer_exp == "QAP":
                color_index = {"A":0, "B":1, "C":2}[train_instance[0]]
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

            elif transfer_exp == "Transfer16OnlyOne":
                color_index = 1 if train_instance in ("_6_", "_11_", "_2_", "_1_", "_5_") else (2 if train_instance in ("_8_", "_4_", "_7_", "_3_") else (0 if train_instance in ("_9_", "_10_", "_12_") else None)) 
                label = "Cluster " + str(color_index+1)
            else:

                color_index = 0
                label = None
                ax.annotate(train_instance, (xi, yi), fontsize='x-small')
            ci = colors[color_index] #color for ith feature 
            ma = markers[color_index]
                
            if label not in used_labels:
                used_labels.add(label)
            else:        
                label = None

            ax.scatter(xi,yi,marker=ma, color=ci, label=label)

        if transfer_exp == "Transfer16OnlyOne":
            handles, labels = plt.gca().get_legend_handles_labels()
            order = [2,0,1]
            ax.legend([handles[idx] for idx in order],[labels[idx] for idx in order], fontsize=8, markerscale=1.0)
        else:
            ax.legend(fontsize=8, markerscale=1.0)
        fig.tight_layout()
        fig.savefig(file_name)
        plt.close()

    # response_array_fit = preprocessing.normalize(np.asarray([el[which_xi_selected_in_response] for el in response_list_averaged_per_train_instance_and_seed]))
    # response_array_predict = preprocessing.normalize(response_list_averaged_per_train_instance)

    response_array_fit = np.asarray([el for el in response_list_averaged_per_train_instance_and_seed])
    response_array_predict = response_list_averaged_per_train_instance



    # # # For debugging python with interactive shell. Start interactive shell.
    # import code
    # code.interact(local=locals())

    plot_MDS(response_array_predict, response_array_predict, np.asarray(target_class_list), save_fig_path+"PCA_only_predict_response_"+transfer_exp+".pdf", method="PCA")
    plot_MDS(response_array_fit, response_array_predict, np.asarray(target_class_list), save_fig_path+"PCA_fit_and_predict_response_"+transfer_exp+".pdf", method="PCA")
    # plot_MDS(response_array_predict, response_array_predict, np.asarray(train_names), save_fig_path+"LDA_only_predict_response_"+transfer_exp+".pdf", method="LDA")
    plot_MDS(response_array_fit, response_array_predict, np.asarray(target_class_list), save_fig_path+"LDA_fit_and_predict_response_"+transfer_exp+".pdf", method="LDA")
