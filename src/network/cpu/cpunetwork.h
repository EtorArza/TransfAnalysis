#pragma once

#include "network.h"

namespace NEAT {

    //---
    //--- CLASS CpuNetwork
    //---
	class CpuNetwork : public Network {
    public:
        NetDims dims;
		std::vector<NetNode> nodes;
		std::vector<NetLink> links;
        double** signature;
        bool signature_initialized=false;

        std::vector<real_t> activations;
        CpuNetwork() {}

        CpuNetwork(const CpuNetwork&  other);

		~CpuNetwork();

		void activate();
        std::vector<real_t> &get_activations(__out std::vector<real_t> &result);
        void set_activations(__in std::vector<real_t> &newacts);

        void clear_noninput();
        void compute_signature();
        void apply_function_to_signature(void (*f)(double* ));
        bool are_signatures_equal(CpuNetwork* other);
        std::vector<real_t> get_noninput(){return activations;};
        void set_noninput(std::vector<real_t> activations){this->activations = activations;};
        void load_sensor(size_t isensor, real_t activation);
        real_t *get_outputs();

        virtual void configure(const NetDims &dims,
                               NetNode *nodes,
                               NetLink *links);

        virtual NetDims get_dims() { return dims; }
	};

}
