#include "mcmf.h"
#include <string.h>
#define ADDITION_VM_PENALTY 50

/**
 * @brief Use Bellman-Ford variation to calculate the shortest path between source and sink.
 * 
 * Each path carries capacity and cost. The Find Cheapest Path (FCP) function primarily
 * uses the cost as the weight for getting a cheapest path that has capacity greater
 * than zero between source and sink.
 * 
 * A queue is introduced to make Bellman-Ford more efficient. Also known as the SPFA.
 * 
 * @param g The flow network represented through a graph.
 * @param source The node index for source node (usually zero).
 * @param sink The node index for sink node.
 * @param out_dist The shortest distance for each node to source node.
 * @param out_prev_edges The edges connects from source to sink. (e.g. The index of edge connects to node u is stored as prev_edge[u])
 * @return true if a path exists.
 */
static bool find_cheapest_path(FlowGraph *g, int source, int sink, int *out_dist, int *out_prev_edges) {
    bool in_queue[MAX_NODES];
    int queue[MAX_NODES];
    int q_head = 0, q_tail = 0;  // Circular queue

    /* Set all distance[node] to infinity except the source */
    for (int i = 0; i < MAX_NODES; i++) {
        out_dist[i] = INF;      // All node distance to source is set to infinity. Reset source later.
        in_queue[i] = false;    // No node in queue yet.
        out_prev_edges[i] = -1;  // No edge is decided for each node yet.
    }
    out_dist[source] = 0;       // Distance to source from source is set to zero.

    /* Enqueue source for kicking off the loop */
    queue[q_tail++] = source;
    in_queue[source] = true;
    
    /* SPFA loop stops no more node's distance to source is updated */
    while (q_head != q_tail) {
        int node_u = queue[q_head++];
        if (q_head >= MAX_NODES) {
            /* Circular queue reset */
            q_head = 0;
        }
        in_queue[node_u] = false;
        /* Get each edge from the source */
        for(int e = g->heads[node_u]; e >= 0; e = g->edges[e].next) {
            Edge edge = g->edges[e];
            int node_v = edge.to;
            int capacity = edge.capacity;
            int cost = edge.cost;

            if (capacity > 0 && out_dist[node_u] + cost < out_dist[node_v]) {
                /* Relax the distance for the "to" node */
                out_dist[node_v] = out_dist[node_u] + cost;
                out_prev_edges[node_v] = e;  // Remember the edge that relaxed node v

                if (!in_queue[node_v]) {
                    queue[q_tail++] = node_v;   // Add to queue for updating its neighbors' distance
                    in_queue[node_v] = true;
                    if (q_tail >= MAX_NODES) {
                        /* Circular queue reset */
                        q_tail = 0;
                    }
                }
            }
        }
    }

    return out_dist[sink] != INF;
}

/**
 * @brief Updates the flow graph to achieve minimum cost and maximum flow between source and sink
 * 
 * Uses the MCMF algorithm to allocate capacities for all nodes between source and sink to achieve
 * the maximum flow (i.e. use as much capacity as possible) while keeps the total cost low.
 * 
 * @param graph The flow network between source and sink.
 * @param source The source node in the graph
 * @param sink The sink node in the graph
 * @return A MCMFRsult object with total flow, and total cost
 */
MCMFResult mcmf_solve(FlowGraph *graph, int source, int sink) {
    MCMFResult result = {
        .total_flow = 0,
        .total_cost =  0
    };
    int distance[MAX_NODES];    // Record the shortest/cheapest path from source to each node
    int prev_edges[MAX_NODES];  // Record the edge that leads the shortest path to each node

    while(find_cheapest_path(graph, source, sink, distance, prev_edges)) {
        /* Calculate bottleneck (i.e. the max flow on the shortest path from source and sink) */
        int bottleneck = INF;
        for (int e = prev_edges[sink]; e >= 0; e = prev_edges[graph->edges[e ^ 1].to]){
            Edge edge = graph->edges[e];
            if (edge.capacity < bottleneck) {
                bottleneck = edge.capacity;
            }
        }

        /* Push capacity through the graph and update reverse edges */
        for (int e = prev_edges[sink]; e >= 0; e = prev_edges[graph->edges[e ^ 1].to]){
            graph->edges[e].capacity -= bottleneck;
            graph->edges[e ^ 1].capacity += bottleneck;
            graph->edges[e].flow += bottleneck;
            /* remove the flow if the counterpart edge was used previously */
            if (graph->edges[e ^ 1].flow > 0) {
                graph->edges[e ^ 1].flow -= bottleneck;
            }
            /* Add penalty to edge betwen pCPU and Sink for adding second VM */
            if (e == prev_edges[sink]) {
                graph->edges[e].cost += ADDITION_VM_PENALTY;
                graph->edges[e ^ 1].cost -= ADDITION_VM_PENALTY;
            }
        }
        
        /* bottleneck is the total amount of flow can be pushed for current path */
        result.total_flow += bottleneck;
        /* distance[sink] give the total cost for connecting source to sink */
        result.total_cost += distance[sink] * bottleneck;
    }

    return result;
}
