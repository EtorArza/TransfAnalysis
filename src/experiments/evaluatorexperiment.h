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
#include "constants.h"
#include "speciesorganism.h"

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
        Organism *best_this_reinitialize = nullptr;

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
            neat_params->N_TIMES_BEST_FITNESS_IMPROVED_TRAIN = 0;
            neat_params->N_ITERATIONS_WITHOUT_FITNESS_IMPROVED = 0;
            neat_params->BEST_FITNESS_THIS_REINITIALIZE = -1e80;
            neat_params->BEST_FITNESS_TRAIN_ALL_TIME = -1e81;


            neat_params->IS_LAST_ITERATION = false;


            Organism *best_all_time = nullptr;


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

                if(gen != 1 && !neat_params->reinitialize_neat_for_next_generation) {
                    pop->next_generation();
                }

                // reinizialize after evaluate. In this manner, evaluate() knows reinizialization will be applied, and tests best network
                neat_params-> reinitialize_neat_for_next_generation = false;
                if (gen % REINITIALIZE_CONTROLLERS_EVERY_N_ITERATIONS == 0 && !neat_params->IS_LAST_ITERATION)
                {
                    neat_params->reinitialize_neat_for_next_generation = true;
                    cout << "reinitialize_message -> REINITITIALIZING" << endl;
                }
                evaluate(best_all_time);

                timer.stop();
                Timer::report();



                if (save_best_network)
                {
                    save_best_network = false;
                    print(gen, false);
                }

                if (neat_params->IS_LAST_ITERATION || neat_params->reinitialize_neat_for_next_generation)
                {
                    print(gen, true);
                }

                if (neat_params->reinitialize_neat_for_next_generation)
                {

                    neat_params->N_TIMES_BEST_FITNESS_IMPROVED_TRAIN = 0;
                    neat_params->N_ITERATIONS_WITHOUT_FITNESS_IMPROVED = 0;
                    neat_params->BEST_FITNESS_THIS_REINITIALIZE = -1e80;
                    int seed = rng.integer();
                    cout << "reinitialize_message -> Reinitializing population with SEED = " << seed << endl;
                    rng_t rng_exp(seed);
                    delete pop;
                    delete env->genome_manager;
                    env->genome_manager = GenomeManager::create();
                    vector<unique_ptr<Genome>> genomes = create_seeds(rng_exp);
                    pop = Population::create(rng_exp, genomes);
                    best_this_reinitialize=nullptr;
                }
            }
            delete pop;
            delete env->genome_manager;
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
                std::cout << "reinitialize_message -> saving in all_controllers genome with hash #" << fittest->genome->hash() << std::endl;
                best_this_reinitialize->write(out);
            }
            else
            {
                std::cout << "reinitialize_message -> saving in top_controllers genome with hash #" << fittest->genome->hash() << std::endl;
                ofstream out(get_fittest_path(generation, "top_controllers"));
                fittest->write(out);
            }
        }

        void evaluate(Organism *&best_all_time) {
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


            for(size_t i = 0; i < norgs; i++) {
                Organism *org = pop->get(i);
                org->eval = evaluations[i];
                if( best_this_reinitialize == nullptr || (org->eval.fitness > best_this_reinitialize->eval.fitness) ) 
                {
                    best_this_reinitialize = org;
                    save_best_network = true;
                }
            }

            // If it is the first iteration, or (best fitness all time improved)
            if (best_all_time == nullptr || neat_params->BEST_FITNESS_THIS_REINITIALIZE > neat_params->BEST_FITNESS_TRAIN_ALL_TIME)
            {
                cout << "BF_THIS_REINITIALIZE, BF_TRAIN_ALL_TIME = " 
                    << neat_params->BEST_FITNESS_THIS_REINITIALIZE << ", " 
                    << neat_params->BEST_FITNESS_TRAIN_ALL_TIME << endl;
                cout << "reinitialize_message -> new_best_found between reinits with hash #" << best_this_reinitialize->genome->hash() << endl;
                neat_params->BEST_FITNESS_TRAIN_ALL_TIME = neat_params->BEST_FITNESS_THIS_REINITIALIZE;
                best_all_time = new SpeciesOrganism(*best_this_reinitialize->genome);;
                SpeciesOrganism *copy = new SpeciesOrganism(*best_all_time->genome);
                fittest = unique_ptr<Organism>(copy);
                fittest->eval.fitness = neat_params->BEST_FITNESS_TRAIN_ALL_TIME;
            }

            timer.stop();
            Genome::Stats gstats = fittest->genome->get_stats();
            cout << "fittest [" << fittest->population_index << "]"
                 << ": fitness=" << fittest->eval.fitness
                 << ", nnodes=" << gstats.nnodes
                 << ", nlinks=" << gstats.nlinks
                 << endl;
        }

        class Population *pop;
        std::unique_ptr<Organism> fittest;
    };

}
