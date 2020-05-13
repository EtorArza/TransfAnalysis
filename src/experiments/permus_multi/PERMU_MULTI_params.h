#include <string>
#include "../../PERMU_params.h"
#pragma once


namespace PERMU_MULTI{

    class params_multi: public PERMU::params{

    public:
        std::vector<std::string> *VECTOR_OF_INSTANCE_PATHS;
    };

};