#include "std.h"
#include "evaluatorexperiment.h"
#include "genomemanager.h"
#include "../permus/permuevaluator.h" 
#include "true_ranking_eval_evaluator.h"
#include "neat.h"
#include "rng.h"
 
using namespace NEAT;

static struct true_ranking_evalInit {
    true_ranking_evalInit() {
        auto create_evaluator = [] () {
            return create_true_ranking_eval_evaluator();
        };
 
        auto create_seeds = [] (rng_t rng_exp) {

            int nhidden;


            nhidden = PERMU::sensor_t::__sensor_N;

            

            return env->genome_manager->create_seed_generation(env->pop_size,
                                                        rng_exp,
                                                        1,
                                                        PERMU::sensor_t::__sensor_N,  // number of input nodes.
                                                        PERMU::output_t::__output_N,  // number output nodes.
                                                        nhidden); // number of default hidden nodes.
        };

        //todo: This is wonky. Should maybe make an explicit static registry func?
        new EvaluatorExperiment("true_ranking_eval", create_evaluator, create_seeds);
    }
} init;
