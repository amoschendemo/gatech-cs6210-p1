#include "graph.h"
#include <string.h>

void graph_init(FlowGraph *g, int nr_nodes) {
    g->nr_edges = 0;
    g->nr_nodes = nr_nodes;
    memset(g->heads, -1, sizeof(g->heads));
}

int graph_add_edge(FlowGraph *g, int u, int v, int capacity, int cost) {
    int forward_edge_idx = g->nr_edges;

    /* Add forward edge from u to v */
    g->edges[forward_edge_idx].to = v;
    g->edges[forward_edge_idx].capacity = capacity;
    g->edges[forward_edge_idx].cost = cost;
    g->edges[forward_edge_idx].flow = 0;
    g->edges[forward_edge_idx].next = g->heads[u];
    g->heads[u] = forward_edge_idx;

    /* Add reverse edge from v to u */
    int reverse_edge_idx = forward_edge_idx + 1;
    g->edges[reverse_edge_idx].to = u;
    g->edges[reverse_edge_idx].capacity = 0;
    g->edges[reverse_edge_idx].cost = -cost;
    g->edges[reverse_edge_idx].flow = 0;
    g->edges[reverse_edge_idx].next = g->heads[v];
    g->heads[v] = reverse_edge_idx;

    g->nr_edges += 2;
    
    return forward_edge_idx;
}