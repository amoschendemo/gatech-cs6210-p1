#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <limits.h>

#define MAX_NODES  1 + 8 + 4 + 1      // Source + VMs + PCPUs + Sink
#define MAX_EDGES (8 + 8 * 4 + 4) * 2 // Double edges
#define INF       INT_MAX

/**
 * @brief Represents a directed edge that connects two nodes.
 * 
 * All edges from the same source node are linked through the next field.
 * The starting point of these edges is stored in the heads array in the
 * FlowGraph. The last edge of a given source node has next equals -1.
 */
typedef struct {
    /* @brief The target node of this edge */
    int to;
    /* @brief The remaining capacity of this edge */
    int capacity;
    /* @brief The cost for flowing through this edge */
    int cost;
    /* @brief The current flow */
    int flow;
    /* @brief The next edge in the adjacency list (-1 when reachs the end) */
    int next;
} Edge;

/**
 * @brief A flow network for supporting Min Cost Max Flow (MCMF) calculation
 * 
 * VMs and PCPUs are stored as nodes. Using edge's capacity to configure 
 * assignment constrains. Then use cost to pioritize the affinity and balance
 * the utilization across PCPUs.
 * 
 * All the edges are stored in one array to avoid dynamic memory allocation.
 * 
 * When adding a new edge to this graph it results a pair of forward edge and 
 * reverse edge stored.
 * 
 * edges[n]/edges[n + 1] is forward edge and its counterpart reverse edge.
 * 
 * heads[u] = index of node u's first edge. edges[index[u]] is the starting
 * edge for node u's adjacency list.
 */
typedef struct {
    /* @brief All the edges are stored in pairs - forward edge is followed by a reverse edge */
    Edge edges[MAX_EDGES];
    /* @brief All the nodes and stores the index for the first edge in the adjacency list */
    int heads[MAX_NODES];
    /* @brief Number of edges */
    int nr_edges;
    /* @brief Number of nodes */
    int nr_nodes;
} FlowGraph;

/**
 * @brief Initialize a new FlowGraph for given number of nodes.
 */
void graph_init(FlowGraph *g, int nr_nodes);

/**
 * @brief Adds an edge that connects node u and v with certain capacity and cost.
 */
int graph_add_edge(FlowGraph *g, int u, int v, int capacity, int cost);

/**
 * @brief Print out the graph for debugging
 */
void print_graph(const FlowGraph *g);

#endif