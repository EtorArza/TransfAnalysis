#pragma once

#include "experiment.h"
#include "genomemanager.h"
#include "network.h"
#include "organism.h"
#include "population.h"
#include "stats.h"
#include "timer.h"
#include "util.h"
#include "Parameters.h"
#include <chrono>
#include <float.h>
namespace NEAT {

//------------------------------
//---
//--- CLASS EvaluatorExperiment
//---
//------------------------------
    class EvaluatorExperiment : public Experiment {
    private:
        std::string get_dir_path() {
            return neat_params->EXPERIMENT_FOLDER_NAME;
        }

        std::string get_fittest_path(int generation, std::string last_dir_name="all_controllers") {
            char buf[1024];
            if (last_dir_name != "top_controllers")
            {
                sprintf(buf, "%s/%s/%s_gen_%09d.controller",
                    neat_params->EXPERIMENT_FOLDER_NAME.c_str(),
                    last_dir_name.c_str(),
                    neat_params->CONTROLLER_NAME_PREFIX.c_str(),
                    generation
                    );
            }
            else
            {
            
            sprintf(buf, "%s/%s/%s_best.controller",
                    neat_params->EXPERIMENT_FOLDER_NAME.c_str(),
                    last_dir_name.c_str(),
                    neat_params->CONTROLLER_NAME_PREFIX.c_str()
                    );
            }

            return buf;
        }

    public:

        bool save_best_network = false;

        typedef std::function<NetworkEvaluator *()> CreateEvaluatorFunc;
        typedef std::function< std::vector<std::unique_ptr<Genome>> (rng_t rng)> CreateSeedsFunc;

        CreateEvaluatorFunc create_evaluator;
        CreateSeedsFunc create_seeds;
        std::unique_ptr<NetworkEvaluator> network_evaluator;

        EvaluatorExperiment(const char *name,
                            CreateEvaluatorFunc create_evaluator_,
                            CreateSeedsFunc create_seeds_)
            : Experiment(name)
            , create_evaluator(create_evaluator_)
            , create_seeds(create_seeds_) {
        }



        virtual ~EvaluatorExperiment() {}



        virtual void run(class rng_t &rng) override {
            using namespace std;

            // network evaluator normally created in run_given_conf_file()
            if (network_evaluator==NULL){
            cerr << "Waring in class in EvaluatorExperiment: Network evalluator initialized in run() method.\n"
            << "It should be initialized by calling run_given_conf_file() in this class." << endl;
            network_evaluator = unique_ptr<NetworkEvaluator>(create_evaluator()); 
            }
            vector<size_t> nnodes;
            vector<size_t> nlinks;
            vector<real_t> fitness;


            if (neat_params->MAX_TRAIN_TIME == -1 && neat_params->MAX_TRAIN_ITERATIONS == -1)
            {
                std::cout << "Error, MAX_TRAIN_TIME or MAX_TRAIN_ITERATIONS required as stopping criteria for training with NEAT." << endl;
                exit(1);
            }

            
            mkdir( get_dir_path() );
            mkdir( get_dir_path() + "/all_controllers" );
            mkdir( get_dir_path() + "/top_controllers" );

            //Create a unique rng sequence for this experiment
            rng_t rng_exp(rng.integer());

            fittest = nullptr;
            env->genome_manager = GenomeManager::create();
            vector<unique_ptr<Genome>> genomes = create_seeds(rng_exp);


            pop = Population::create(rng_exp, genomes);
            
            
            int gen = 0;
            neat_params->BEST_FITNESS_TRAIN = -DBL_MAX;
            neat_params->N_TIMES_BEST_FITNESS_IMPROVED_TRAIN = 0;
            neat_params->N_ITERATIONS_WITHOUT_FITNESS_IMPROVED = 0;


            neat_params->IS_LAST_ITERATION = false;


            #ifdef HIPATIA
            for(double progress = 0; !neat_params->IS_LAST_ITERATION; progress = ((double) get_runtime_hipatia() / (double) neat_params->MAX_TRAIN_TIME)) 
            #else
            for(double progress = 0; !neat_params->IS_LAST_ITERATION; progress = ((double) neat_params->global_timer.toc() / (double) neat_params->MAX_TRAIN_TIME)) 
            #endif
            {

                if (   progress >= 1.0 || (gen >= neat_params->MAX_TRAIN_ITERATIONS && neat_params->MAX_TRAIN_ITERATIONS != -1)   )
                {
                    neat_params->IS_LAST_ITERATION = true;
                }
                
                gen++;
                cout << "\n ---------------------------------------------------- \n\n";
                cout << "Gen " << gen-1 << " / " << neat_params->MAX_TRAIN_ITERATIONS << ", progress: " << progress << endl;	
                #ifdef HIPATIA
                cout << "Time left:" << ((double) neat_params->MAX_TRAIN_TIME - get_runtime_hipatia()) / 60.0 / 60.0 << "h" << endl;
                #else
                cout << "Time left:" << ((double) neat_params->MAX_TRAIN_TIME - neat_params->global_timer.toc()) / 60.0 / 60.0 << "h" << endl;
                #endif
                

                static Timer timer("epoch");
                timer.start();

                if(gen != 1) {
                    pop->next_generation();
                }

                evaluate();

                timer.stop();
                Timer::report();



                if (save_best_network)
                {
                    save_best_network = false;
                    print(gen, false);
                }

                if (neat_params->IS_LAST_ITERATION)
                {
                    print(gen, true);
                }
            }

            {
                Genome::Stats gstats = fittest->genome->get_stats();
                fitness.push_back(fittest->eval.fitness);
                nnodes.push_back(gstats.nnodes);
                nlinks.push_back(gstats.nlinks);
            }
            delete pop;
            delete env->genome_manager;

            cout << "fitness stats: " << stats(fitness) << endl;
            cout << "nnodes stats: " << stats(nnodes) << endl;
            cout << "nlinks stats: " << stats(nlinks) << endl;

        }

        virtual void run_given_conf_file(std::string conf_file_path) override{
            neat_params->load_global_params(conf_file_path);
            network_evaluator = unique_ptr<NetworkEvaluator>(create_evaluator());
            network_evaluator->neat_params = neat_params;
            network_evaluator->run_given_conf_file(conf_file_path);
        }


    private:
        void print(int generation, bool is_last_gen = false) {
            using namespace std;
            if (!is_last_gen)
            {
                ofstream out(get_fittest_path(generation, "all_controllers"));
                fittest->write(out);
            }
            else
            {
                ofstream out(get_fittest_path(generation, "top_controllers"));
                fittest->write(out);
            }
        }

        void evaluate() {
            using namespace std;

            static Timer timer("evaluate");
            timer.start();

            size_t norgs = pop->size();
            Network *nets[norgs];
            for(size_t i = 0; i < norgs; i++) {
                nets[i] = pop->get(i)->net.get();
            }
            OrganismEvaluation evaluations[norgs];
            //auto tmp_params = network_evaluator->
            cout << "execute()" << endl;
            network_evaluator->execute(nets, evaluations, norgs);
            cout << "execute() end" << endl;

            Organism *best = nullptr;
            for(size_t i = 0; i < norgs; i++) {
                Organism *org = pop->get(i);
                org->eval = evaluations[i];
                if( !best || (org->eval.fitness > best->eval.fitness) ) {
                    best = org;
                }
            }

            timer.stop();

            // Fittest is not evaluated.
            if(!fittest || (best->eval.fitness > fittest->eval.fitness+ 1e-60)) {
                save_best_network = true;
                fittest = pop->make_copy(best->population_index);
            }

            Genome::Stats gstats = fittest->genome->get_stats();
            cout << "fittest [" << fittest->population_index << "]"
                 << ": fitness=" << neat_params->BEST_FITNESS_TRAIN
                 << ", error=" << fittest->eval.error
                 << ", nnodes=" << gstats.nnodes
                 << ", nlinks=" << gstats.nlinks
                 << endl;
        }

        class Population *pop;
        std::unique_ptr<Organism> fittest;
    };

}
