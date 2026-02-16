#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "vm_types.h"
#include "graph.h"
#include "mcmf.h"

typedef struct {
    int vm_to_pcpu[MAX_VMS];   /* result: vm i assigned to pcpu vm_to_pcpu[i] */
    int num_assigned;
    int total_cost;
} Schedule;

Schedule compute_schedule(const SystemState *state);

#endif