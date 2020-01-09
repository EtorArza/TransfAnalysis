#pragma once
#include "std.h" // Must be included first. Precompiled header with standard library includes.

#include "loadnetworkfromfile.h"
#include "cpunetwork.h"
#include "Tools.h"
#include <string>
#include "assert.h"
#include <unordered_map>
#include "innovgenome.h"
#include "innovnodegene.h"
#include "innovlinkgene.h"
#include "trait.h"
#include "evaluatorexperiment.h"
#include "neat.h"
#include "neattypes.h"
#include "cpunetwork.h"

#include "experiment.h"
#include "genomemanager.h"
#include "network.h"
#include "organism.h"
#include "population.h"


// this function is more or less working, but the one below is definitely better. So use the one below.
// void load_network(string filename)
// {

//     // str.find(str2) --> str contains str2
//     std::vector<std::string> string_vector = read_lines_from_file(filename);


//     std::unordered_map <string, int> from_name_to_index;



//     NEAT::NetDims dims;
//     memset(&dims, 0, sizeof(dims));
//     int node_counter = 0;
//     // load dims and load node names
//     for (size_t i = 0; i < string_vector.size(); i++)
//     {
//         string line = string_vector[i];
//         if (line.find("node") != line.npos)
//         {
//             dims.nnodes.all++;
//             int node_type = line.back() - '0';
//             switch (node_type)
//             {
//             case 0: // bias node
//                 dims.nnodes.bias++;
//                 dims.nnodes.input++;
//                 break;
//             case 1: // sensor node
//                 dims.nnodes.sensor++;
//                 dims.nnodes.input++;
//                 break;
//             case 2: // output node
//                 dims.nnodes.output++;
//                 dims.nnodes.noninput++;

//                 break;
//             case 3: // hidden node
//                 dims.nnodes.hidden++;
//                 dims.nnodes.noninput++;
//                 break;
            
//             default:
//                 break;
//             }
//             std::vector<std::string> splt_line = split(line, ' ');
//             std::pair <std::string,int> item_pair(splt_line[1], node_counter);
//             from_name_to_index.insert(item_pair);
//             node_counter++;
//         }
//         else if (line.find("gene") != line.npos)
//         {
//             dims.nlinks++;
//         }
//     }



//     NEAT::NetLink netlinks[dims.nlinks];
//     size_t node_nlinks[dims.nnodes.all] = {0}; // saves the number of outgoing links of each node
//     // load links
//     int link_idx = 0;
//     for (size_t line_idx = 0; line_idx < string_vector.size(); line_idx++)
//     {
//         string line = string_vector[line_idx];
//         if (line.find("gene") != line.npos)
//         {
//             std::vector<std::string> splt_line = split(line, ' ');
            
//             netlinks[link_idx].in_node_index = from_name_to_index[splt_line[2]];// stoi(splt_line[2]);
//             netlinks[link_idx].out_node_index = from_name_to_index[splt_line[3]];// stoi(splt_line[3]);
//             netlinks[link_idx].weight = stof(splt_line[4]);
//             node_nlinks[netlinks[link_idx].out_node_index]++;
//             link_idx++;
//         }
//     }

//     //---
//     //--- Determine layout of links for each node in sorted array
//     //---
//     NEAT::NetNode netnodes[dims.nnodes.all];
//     netnodes[0].incoming_start = 0;
//     netnodes[0].incoming_end = node_nlinks[0];
//     for(size_t i = 1; i < dims.nnodes.all; i++) {
//         NEAT::NetNode &prev = netnodes[i-1];
//         NEAT::NetNode &curr = netnodes[i];

//         curr.incoming_start = prev.incoming_end;
//         curr.incoming_end = curr.incoming_start + node_nlinks[i];
//     }
//     assert(netnodes[dims.nnodes.all - 1].incoming_end == dims.nlinks);



//     //---
//     //--- Create sorted links
//     //---
//     memset(node_nlinks, 0, sizeof(size_t) * dims.nnodes.all);
//     NEAT::NetLink netlinks_sorted[dims.nlinks];
//     for(size_t i = 0; i < dims.nlinks; i++) {
//         NEAT::NetLink &netlink = netlinks[i];
//         size_t inode = netlink.out_node_index;
//         size_t isorted = netnodes[inode].incoming_start + node_nlinks[inode]++;
//         netlinks_sorted[isorted] = netlink;
//     }


//     //---
//     //--- Configure the net
//     //---
//     NEAT::CpuNetwork net;
//     net.configure(dims, netnodes, netlinks_sorted);

// }

namespace NEAT
{
CpuNetwork load_network(string filename)
{

    // str.find(str2) --> str contains str2
    std::vector<std::string> string_vector = read_lines_from_file(filename);

    std::vector<Trait> traits;
    std::vector<InnovNodeGene> nodes;
    std::vector<InnovLinkGene> links;



    // load dims
    NEAT::NetDims dims;
    memset(&dims, 0, sizeof(dims));
    int node_counter = 0;
    // load dims and load node names
    for (size_t i = 0; i < string_vector.size(); i++)
    {
        string line = string_vector[i];
        if (line.find("node") != line.npos)
        {
            dims.nnodes.all++;
            int node_type = line.back() - '0';
            switch (node_type)
            {
            case 0: // bias node
                dims.nnodes.bias++;
                dims.nnodes.input++;
                break;
            case 1: // sensor node
                dims.nnodes.sensor++;
                dims.nnodes.input++;
                break;
            case 2: // output node
                dims.nnodes.output++;
                dims.nnodes.noninput++;

                break;
            case 3: // hidden node
                dims.nnodes.hidden++;
                dims.nnodes.noninput++;
                break;

            default:
                break;
            }
            node_counter++;
        }
        else if (line.find("gene") != line.npos)
        {
            dims.nlinks++;
        }
    }

    // NEAT::NetNode* netnodes = new NEAT::NetNode[dims.nnodes.all];
    // NEAT::NetLink* netlinks = new NEAT::NetLink[dims.nlinks];

    // size_t netnode_index = 0;
    // size_t netlink_index = 0;

    // load trait, nodes and and links
    for (size_t i = 0; i < string_vector.size(); i++)
    {
        string line = string_vector[i];
        if (line.find("trait") != line.npos)
        {
            traits.push_back(Trait(line));
        }
        else if (line.find("node") != line.npos)
        {
            nodes.push_back(InnovNodeGene(line));
        }
        else if (line.find("gene") != line.npos)
        {
            links.push_back(InnovLinkGene(line));
        }
    }

    InnovGenome g(traits, nodes, links);

    env->genome_manager = NEAT::GenomeManager::create();

    

    // Organism *org;
    // org = new Organism(g);
    CpuNetwork net;
    g.init_phenotype(&net);
    return net;
    // org->population_index = 10000;
    //org->write(cout);
    // return org;
    // return net;
    // std::unique_ptr<NetworkEvaluator> network_evaluator;
    // network_evaluator = unique_ptr<NetworkEvaluator>(NEAT::create_evaluator());
}
} // namespace NEAT