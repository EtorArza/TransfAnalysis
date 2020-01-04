#pragma once

namespace NEAT {

    class Experiment {
    public:
        static std::vector<std::string> get_names();
        static Experiment *get(const char *name);

    private:
        static std::map<std::string, Experiment*> *experiments;

    public:
        virtual ~Experiment();

        virtual void run(class rng_t &rng) = 0;
        virtual void run_given_conf_file(std::string conf_file_path) = 0;


    protected:
        Experiment(const char *name);

    private:
        Experiment() {}

        const char *name;
    };
}
