#include "std.hxx"

#include "parameters.h"
#include "permuevaluator.h"
#include "map.h"
#include "network.h"
#include "networkexecutor.h"
#include "resource.h"
#include <assert.h>

//#define Truncate_Seq 2

#define Max_Seq_Len 3
#define Max_Maze_Len 26

using namespace std;

namespace NEAT
{



struct Config
{
    uchar width;
    uchar height;
    bool wall[Max_Maze_Len * Max_Maze_Len];
    struct Trial
    {
        uchar seqlen;
        real_t seq[Max_Seq_Len];
        ushort max_steps;
        uchar max_dist;
        uchar dist_map[Max_Maze_Len * Max_Maze_Len];
    };
    ushort ntrials;
    Trial trials[];
};


static void create_config(__out Config *&config_, __out size_t &len_)
{
    Map map = parse_map(find_resource("maze.map"));

    Config config;
    config.width = map.width;
    config.height = map.height;
    assert(config.width * config.height <= sizeof(config.wall));

    config.ntrials = 0;

    memset(config.wall, 0, sizeof(bool) * config.width * config.height);

    vector<Config::Trial> trials;



#ifdef Truncate_Seq
                for (size_t i = Truncate_Seq; i < seq.length(); i++)
                {
                    trial.seq[i] = 0.5;
                }
#endif


    for (size_t i = 0; i < config.ntrials; i++)
    {
        Config::Trial &trial = trials[i];
        trial.max_steps = 2 + 3 * trial.seqlen;
    }
    memcpy(config_->trials, trials.data(), sizeof(Config::Trial) * config.ntrials);
}

struct Evaluator
{
    typedef NEAT::Config Config;
    const Config *config;
    bool terminated;
    ushort it;
    ushort max_it;
    OrganismEvaluation eval;

    // constructor
    __net_eval_decl Evaluator(const Config *config_): config(config_)
    {
        it = 0;
        max_it = 10;
        terminated = false;
        eval.error = 0.0;
        eval.fitness = 0.0;
    }

    // Check if evaluation is terminated. If it is, __net_eval_decl OrganismEvaluation result() is called.
    __net_eval_decl bool next_step()
    {
        return !terminated;
    }

    // Clear non-input latent variables. (for example, the states of previous iterations on a recurrent net)
    // This function is called before each iteration, so it could potentially be used to compute the necessary info.
    __net_eval_decl bool clear_noninput()
    {
        return false; // trial_step == 1;
    }

    // load the sensory input
    __net_eval_decl real_t get_sensor(node_size_t sensor_index)
    {
        switch (sensor_t(sensor_index))
        {
        case sensor_relat_f:
            return 0.0;
        case sensor_time:
            return 0.0;
        case sensor_rand:
            return 0.0;
        case sensor_relat_spars:
            return 0.0;
        case sensor_is_local_optima:
            return 0.0;
        case sensor_ham_dist_from_theoneabove:
            return 0.0;
        default:
#ifdef ENABLE_CUDA
            return 0.0;
#else
            abort();
#endif
        }
    }

    // load the outputs, and assign fitness if it it is the last iteration.
    __net_eval_decl void evaluate(real_t *output)
    {
        if (!terminated)
        {
            if (output[ls_nothing_move] < -CUTOFF_0){
                //local search
            }else if(output[ls_nothing_move] > CUTOFF_0){
                //movememnt
            }
        }else{
            eval.fitness = 10.0;
            eval.error = eval.fitness;
        }
    }

    __net_eval_decl OrganismEvaluation result()
    {
        return eval;
    }
};

class PermuEvaluator : public NetworkEvaluator
{
    NetworkExecutor<Evaluator> *executor;

public:
    PermuEvaluator()
    {
        executor = NetworkExecutor<Evaluator>::create();
        Evaluator::Config *config;
        size_t configlen;
        create_config(config, configlen);
        executor->configure(config, configlen);
        free(config);
    }

    ~PermuEvaluator()
    {
        delete executor;
    }

    virtual void execute(class Network **nets_,
                         class OrganismEvaluation *results,
                         size_t nnets)
    {
        executor->execute(nets_, results, nnets);
    }
};

class NetworkEvaluator *create_permu_evaluator()
{
    return new PermuEvaluator();
}

} // namespace NEAT
