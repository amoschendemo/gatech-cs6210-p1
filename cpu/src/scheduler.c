#include <string.h>
#include "vm_types.h"
#include "graph.h"
#include "mcmf.h"
#include "scheduler.h"

/**
 * VM PCPU affinity provides edge cost at 0. Assigning different PCPU
 * incurs a migration penalty. The cost is 50% of a fully utilized CPU.
 */
#define MIGRATION_PENALTY 50

Schedule compute_schedule(const SystemState *state) {
    FlowGraph g;
    Schedule schedule;
    memset(&schedule, -1, sizeof(Schedule));

    int nr_vms = state->nr_vms;
    int nr_pcpus = state->nr_pcpus;

    /* Node Indexes */
    int source = 0;
    int vm_base = 1;
    int pcpu_base = vm_base + nr_vms;
    int sink = pcpu_base + nr_pcpus;

    graph_init(&g, sink + 1);

    /* Define Source to each VM */
    for (int i = 0; i < nr_vms; i++) {
        graph_add_edge(&g, source, vm_base + i, 1, 0);
    }

    /* Define VM to each PCPU */
    for (int i = 0; i < nr_vms; i++) {
        for (int j = 0; j < nr_pcpus; j++) {
            int affinity_cost = (state->vms[i].current_pcpu == state->pcpus[j].id) ? 0 : MIGRATION_PENALTY;
            int pcpu_utilization_cost = (int) state->pcpus[j].utilization_rate;
            int cost = affinity_cost + pcpu_utilization_cost;
            graph_add_edge(&g, vm_base + i, pcpu_base + j, 1, cost);
        }
    }

    /* Define PCPU to Sink */
    for (int j = 0; j < nr_pcpus; j++) {
        graph_add_edge(&g, pcpu_base + j, sink, 1, 0);
    }

    MCMFResult result = mcmf_solve(&g, source, sink);
    schedule.total_cost = result.total_cost;
    schedule.num_assigned = result.total_flow;

    /* Extract VM to PCPU assignment */
    for (int i = 0; i < nr_vms; i++) {
        for(int e = g.heads[vm_base + i]; e >= 0; e = g.edges[g.heads[vm_base + i]].next) {
            Edge edge = g.edges[e];
            if (edge.to >= pcpu_base && edge.flow > 0 && edge.to < sink) {
                schedule.vm_to_pcpu[i] = state->pcpus[edge.to - pcpu_base].id;
                break;
            }
        }
    }

    return schedule;
}