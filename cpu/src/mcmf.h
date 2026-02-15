#ifndef MCMF_H
#define MCMF_H

#include "graph.h"
#include <stdbool.h>

typedef struct {
    int total_flow;
    int total_cost;
} MCMFResult;

MCMFResult mcmf_solve(FlowGraph *graph, int source, int sink);

#endif