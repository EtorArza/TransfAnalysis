#include "std.h"
#include "evaluatorexperiment.h"
#include "genomemanager.h"
#include "real_func_evaluator.h" 
#include "neat.h"
#include "rng.h"
 
using namespace NEAT;

static struct real_funcInit {
    real_funcInit() {
        auto create_evaluator = [] () {
            return create_real_func_evaluator();
        };
 
        auto create_seeds = [] (rng_t rng_exp) {

            int nhidden;


                nhidden = __sensor_N;


            return env->genome_manager->create_seed_generation(env->pop_size,
                                                        rng_exp,
                                                        1,
                                                        __sensor_N,  // number of input nodes.
                                                        __output_N,  // number output nodes.
                                                        nhidden); // number of default hidden nodes.
        };

        //todo: This is wonky. Should maybe make an explicit static registry func?
        new EvaluatorExperiment("real_func", create_evaluator, create_seeds);
    }
} init;
