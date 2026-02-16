#include <stdio.h>
#include <string.h>
#include "graph.h"

void print_graph(const FlowGraph *g) {
    printf("Edges:\n");
    for (int i = 0; i < g->nr_edges; i++) {
        printf("%d: {to: %d, capacity: %d, cost: %d, flow: %d, next: %d}\n", i, g->edges[i].to, g->edges[i].capacity, g->edges[i].cost, g->edges[i].flow, g->edges[i].next);
    }
    printf("Heads:\n");
    for (int i = 0; i < g->nr_nodes; i++) {
        printf("%d: %d\n", i, g->heads[i]);
    }
}

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