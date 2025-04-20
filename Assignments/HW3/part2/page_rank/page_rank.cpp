#include "page_rank.h"

#include <cmath>
#include <cstdlib>
#include <omp.h>

#include "../common/graph.h"

// #include <iostream>
// page_rank --
//
// g:           graph to process (see common/graph.h)
// solution:    array of per-vertex vertex scores (length of array is num_nodes(g))
// damping:     page-rank algorithm's damping parameter
// convergence: page-rank algorithm's convergence threshold
//
void page_rank(Graph g, double *solution, double damping, double convergence)
{

    // initialize vertex weights to uniform probability. Double
    // precision scores are used to avoid underflow for large graphs
    // print_graph(g);
    // std::cout << *(outgoing_begin(g, 1))  << std::endl;
    int nnodes = num_nodes(g);
    double equal_prob = 1.0 / nnodes;
    // double* score_old = (double*)malloc(sizeof(double) * nnodes);
    double* score_old = new double[g->num_nodes];
    // double* score_new = (double*)malloc(sizeof(double) * nnodes);
    // #pragma omp parallel for
    for (int i = 0; i < nnodes; ++i)
    {
        score_old[i] = equal_prob;
    }

    // score_old[vi] = 1/nnodes;
    bool converged = false;
    // int iter = 0, max_iter = 1000;

    while (!converged) {

      double dangling_sum = 0.0;

      // score_new[vi] = sum over all nodes vj reachable from incoming edges
      //                    { score_old[vj] / number of edges leaving vj  }
      // #pragma omp parallel for reduction(+:dangling_sum)
      for(int i = 0; i < nnodes; i++){
        if (outgoing_size(g, i) == 0){
          dangling_sum += score_old[i];
        }
      }

      #pragma omp parallel for
      for(int i = 0; i < nnodes; i++){
        double sum = 0.0;
        // std::cout << i << std::endl;
        for(int j = 0; j < incoming_size(g, i); j++){
          const Vertex *vj = incoming_begin(g, i) + j;

          // std::cout << *vj << std::endl;
          if (outgoing_size(g, *vj) > 0)
            sum += score_old[*vj] / outgoing_size(g, *vj);
        }

        // score_new[vi] = (damping * score_new[vi]) + (1.0-damping) / nnodes;
        // score_new[vi] += sum over all nodes v in graph with no outgoing edges
        //                  { damping * score_old[v] / nnodes }
        solution[i] = (damping * sum) + (1.0 - damping) / nnodes + (damping * dangling_sum / nnodes);
        
        // solution[i] = (damping * sum) + (1.0 - damping) / nnodes;
        // std::cout << i << std::endl;
      } 
      
      // global_diff = sum over all nodes vi { abs(score_new[vi] - score_old[vi]) };
      // std::cout << '1' << std::endl;
      double global_diff = 0.0;
      for(int i = 0; i < nnodes; i++){
        global_diff += fabs(solution[i] - score_old[i]);
        score_old[i] = solution[i];
      }

      // converged = (global_diff < convergence)
      if ( global_diff < convergence )
        converged = true;
    }
    free(score_old);
    /*
       For PP students: Implement the page rank algorithm here.  You
       are expected to parallelize the algorithm using openMP.  Your
       solution may need to allocate (and free) temporary arrays.

       Basic page rank pseudocode is provided below to get you started:

       // initialization: see example code above
       score_old[vi] = 1/nnodes;

       while (!converged) {

         // compute score_new[vi] for all nodes vi:
         score_new[vi] = sum over all nodes vj reachable from incoming edges
                            { score_old[vj] / number of edges leaving vj  }
         score_new[vi] = (damping * score_new[vi]) + (1.0-damping) / nnodes;

         score_new[vi] += sum over all nodes v in graph with no outgoing edges
                            { damping * score_old[v] / nnodes }

         // compute how much per-node scores have changed
         // quit once algorithm has converged

         global_diff = sum over all nodes vi { abs(score_new[vi] - score_old[vi]) };
         converged = (global_diff < convergence)
       }

     */
}
