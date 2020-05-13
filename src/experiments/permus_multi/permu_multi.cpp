#include "std.h"
#include "evaluatorexperiment.h"
#include "genomemanager.h"
#include "permu_multi_evaluator.h" 
#include "neat.h"
#include "rng.h"
 
using namespace NEAT;

static struct Permu_multiInit {
    Permu_multiInit() {
        auto create_evaluator = [] () {
            return NEAT::create_permu_multi_evaluator();
        };
 
        auto create_seeds = [] (rng_t rng_exp) {

            int nhidden;


            nhidden = PERMU::__sensor_N;

            

            return env->genome_manager->create_seed_generation(env->pop_size,
                                                        rng_exp,
                                                        1,
                                                        PERMU::__sensor_N,  // number of input nodes.
                                                        PERMU::__output_N,  // number output nodes.
                                                        nhidden); // number of default hidden nodes.
        };

        //todo: This is wonky. Should maybe make an explicit static registry func?
        new EvaluatorExperiment("permu_multi", create_evaluator, create_seeds);
    }
} init;
