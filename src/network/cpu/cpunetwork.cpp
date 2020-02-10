/*
 Copyright 2001 The University of Texas at Austin

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#ifndef ENABLE_CUDA

#include "std.h" // Must be included first. Precompiled header with standard library includes.
#include "cpunetwork.h"
#include "neat.h"
#include "util.h"
#include <assert.h>
#include "networkexecutor.h"
#include "Parameters.h"
#include "Tools.h"

using namespace NEAT;
using namespace std;



Network *Network::create() {
    return new CpuNetwork();
}

// copy constructor
CpuNetwork::CpuNetwork(const CpuNetwork&  other){ 
    this->activations = std::vector<real_t>(other.activations);

    this->dims = other.dims;
    this->links = other.links;
    this->nodes = other.nodes;
    this->response = other.response;
    this->response_is_being_recorded = other.response_is_being_recorded;
    this->samples_response = other.samples_response;
    this->output_copy = other.output_copy;
    this->function_to_injectivize_output = other.function_to_injectivize_output;
}

CpuNetwork::~CpuNetwork()
{
    if (signature_initialized)
    {
        for (int i = 0; i < NETWORK_SIGNATURE_SIZE; i++)
        {
            delete[] this->signature[i];
        }
        delete[] this->signature;
        signature_initialized=false;
    }
}

void CpuNetwork::apply_function_to_signature(void (*f)(double* )){
    for (int i = 0; i < NETWORK_SIGNATURE_SIZE; i++)
    {
        f(signature[i]);
    }
}

void CpuNetwork::compute_signature(){
    if(!signature_initialized)
    {
        signature_initialized = true;
        signature = new double*[NETWORK_SIGNATURE_SIZE];
        for (int i = 0; i < NETWORK_SIGNATURE_SIZE; i++)
        {
            signature[i] = new double[this->dims.nnodes.output];
        }
    }

    RandomNumberGenerator rng = RandomNumberGenerator();
    rng.seed(2);
    clear_noninput();

    for (int i = 0; i < NETWORK_SIGNATURE_SIZE; i++)
    {
        for (int j = 0; j < dims.nnodes.input; j++)
        {
            load_sensor(j, rng.random_0_1_double() * 2.0 - 1.0);
        }
        this->activate();
        copy_array(signature[i], this->get_outputs(), dims.nnodes.output);
    }
    clear_noninput();
}

bool CpuNetwork::are_signatures_equal(CpuNetwork* other){
    
    if(!(this->signature_initialized && other->signature_initialized))
    {
        cout << "Error, signature comparison without signature initialization" << endl;
        exit(1);
    }
    //cout << count_n_dif_matrix_items_double(this->signature, other->signature, this->dims.nnodes.output, NETWORK_SIGNATURE_SIZE) << " ";
    return count_n_dif_matrix_items_double(this->signature, other->signature, this->dims.nnodes.output, NETWORK_SIGNATURE_SIZE) == 0;
}


// Requires nodes to be sorted by type: BIAS, SENSOR, OUTPUT, HIDDEN
void CpuNetwork::configure(const NetDims &dims_,
                           NetNode *nodes_,
                           NetLink *links_) {
    this->dims = dims_;

    nodes.resize(dims.nnodes.all);
    for(size_t i = 0; i < dims.nnodes.all; i++) {
        nodes[i] = nodes_[i];
    }

    links.resize(dims.nlinks);
    for(size_t i = 0; i < dims.nlinks; i++) {
        links[i] = links_[i];
    }

    activations.resize(dims.nnodes.all);
    for(size_t i = 0; i < dims.nnodes.bias; i++) {
        activations[i] = 1.0;
    }
    for(size_t i = dims.nnodes.bias; i < dims.nnodes.all; i++) {
        activations[i] = 0.0;
    }
}

void CpuNetwork::clear_noninput() {
    std::fill(activations.begin()+dims.nnodes.input, activations.end(), 0);
}

void CpuNetwork::load_sensor(size_t isensor,
                             real_t activation) {
    activations[dims.nnodes.bias + isensor] = activation;
}




real_t *CpuNetwork::get_outputs() {
    return activations.data() + dims.nnodes.input;
}

void CpuNetwork::activate() {
    real_t act_other[dims.nnodes.all];

    //Copy only input activation state.
    memcpy(act_other,
           activations.data(),
           sizeof(real_t) * dims.nnodes.input);

    real_t *act_curr = activations.data(), *act_new = act_other;
    //cout << "--" << endl;
    for(size_t icycle = 0; icycle < NACTIVATES_PER_INPUT; icycle++) {

        for(size_t i = dims.nnodes.input; i < dims.nnodes.all; i++) {
            NetNode &node = nodes[i];

            real_t sum = 0.0;
            for(size_t j = node.incoming_start; j < node.incoming_end; j++) {
                NetLink &link = links[j];
                sum += link.weight * act_curr[link.in_node_index];
                //cout << "from=" << (link.in_node_index) << ", to=" << (i) << ", weight=" << link.weight << ", act[from]=" << act_curr[link.in_node_index] << ", partial=" << link.weight * act_curr[link.in_node_index] << ", sum=" << sum << endl;
            }

            // act_new[i] = NEAT::fsigmoid(sum, 4.924273, 2.4621365);  // Sigmoidal activation.
            act_new[i] = NEAT::ftanh(sum);  // tanh activation function.
        }

        std::swap(act_curr, act_new);
    }

    if(act_curr != activations.data()) {
        // If an odd number of cycles, we have to copy non-input data
        // of act_other back into activations.
        memcpy(activations.data() + dims.nnodes.input,
               act_other + dims.nnodes.input,
               sizeof(real_t) * (dims.nnodes.all - dims.nnodes.input));
    }

    if(response_is_being_recorded){
        samples_response[0]++;
        copy_array(output_copy, this->get_outputs(), dims.nnodes.output);
        this->function_to_injectivize_output(output_copy);
        sum_arrays(response, response, output_copy, dims.nnodes.output);
    }
}

vector<real_t> &CpuNetwork::get_activations(__out vector<real_t> &result) {
    return result = activations;
}

void CpuNetwork::set_activations(__in vector<real_t> &newacts) {
    activations = newacts;
}

void CpuNetwork::start_recording_response(void (*function_to_injectivize_output)(double* ))
{
    this->function_to_injectivize_output = function_to_injectivize_output;
    if (response==NULL && !response_is_being_recorded)
    {
        response = new double[dims.nnodes.output];
        output_copy = new double[dims.nnodes.output];
        samples_response = new int[1];
        samples_response[0] = 0.0;
    }
    response_is_being_recorded = true;
    set_array_to_value(response, 0.0, dims.nnodes.output);
    set_array_to_value(output_copy, 0.0, dims.nnodes.output);

}

void CpuNetwork::return_average_response_and_stop_recording(double* result)
{
    if (response==NULL || response_is_being_recorded==false)
    {
        cout << "ERROR: return response was requested while response not initialized." << endl;
        exit(1);
    }
    
    multiply_array_with_value(response, 1.0 / (double) samples_response[0], dims.nnodes.output);
    copy_array(result, response, dims.nnodes.output);
    delete[] response;
    delete[] samples_response;
    delete[] output_copy;
    response=NULL;
    samples_response=NULL;
}


#endif // ENABLE_CUDA
