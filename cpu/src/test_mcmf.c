#include <assert.h>
#include <stdio.h>
#include "graph.h"
#include "mcmf.h"

static void test_graph_can_init() {
    FlowGraph g;
    
    graph_init(&g, 0);
    assert(g.nr_edges == 0);
    assert(g.nr_nodes == 0);

    graph_init(&g, 8);
    assert(g.nr_edges == 0);
    assert(g.nr_nodes == 8);

    for(int i = 0; i < g.nr_nodes; i++) {
        assert(g.heads[i] == -1);
    }
    
    printf("  PASS: test_graph_can_init\n");
}

static void test_graph_can_add_edge() {
    FlowGraph g;
    int u = 0;
    int v = 1;
    int capacity = 10;
    int cost = 5;
    int nr_nodes = 2;
    graph_init(&g, nr_nodes);
    graph_add_edge(&g, u, v, capacity, cost);
    assert(g.nr_edges == 2);
    assert(g.nr_nodes == 2);
    assert(g.heads[u] == 0);
    assert(g.heads[v] == 1);
    /* Forward edge */
    assert(g.edges[0].to == v);
    assert(g.edges[0].capacity == capacity);
    assert(g.edges[0].cost == cost);
    assert(g.edges[0].flow == 0);
    assert(g.edges[0].next == -1);
    /* Reverse edge */
    assert(g.edges[1].to == u);
    assert(g.edges[1].capacity == 0);
    assert(g.edges[1].cost == -cost);
    assert(g.edges[1].flow == 0);
    assert(g.edges[1].next == -1);

    printf("  PASS: test_graph_can_add_edge\n");
}

static void test_mcmf_can_map_one_vm_to_one_cpu() {
    FlowGraph g;
    int nr_nodes = 4;
    graph_init(&g, nr_nodes);

    int source = 0;
    int vm_1 = 1;
    int pcpu_1 = 2;
    int sink = 3;
    int capacity = 1;
    int cost = 1;
    graph_add_edge(&g, source, vm_1  , capacity, 0   );
    graph_add_edge(&g, vm_1  , pcpu_1, capacity, cost);
    graph_add_edge(&g, pcpu_1, sink  , capacity, 0   );

    MCMFResult result = mcmf_solve(&g, source, sink);
    assert(result.total_cost == 1);
    assert(result.total_flow == 1);

    print_graph(&g);

    /* source -> vm_1 */
    assert(g.edges[g.heads[source]].to       == vm_1);
    assert(g.edges[g.heads[source]].capacity == 0   );
    assert(g.edges[g.heads[source]].flow     == 1   );
    assert(g.edges[g.heads[source]].cost     == 0   );
    assert(g.edges[g.heads[source]].next     == -1  );
    /* vm_1 -> source (i.e reverse) */
    assert(g.edges[g.heads[source] ^ 1].to       == source);
    assert(g.edges[g.heads[source] ^ 1].capacity == 1     );
    assert(g.edges[g.heads[source] ^ 1].flow     == 0     );
    assert(g.edges[g.heads[source] ^ 1].cost     == 0    );
    assert(g.edges[g.heads[source] ^ 1].next     == -1    );

    /* vm_1 -> pcpu_1 */
    assert(g.edges[g.heads[vm_1]].to       == pcpu_1               );
    assert(g.edges[g.heads[vm_1]].capacity == 0                    );
    assert(g.edges[g.heads[vm_1]].flow     == 1                    );
    assert(g.edges[g.heads[vm_1]].cost     == 1                    );
    assert(g.edges[g.heads[vm_1]].next     == (g.heads[source] ^ 1));
    /* pcpu_1 -> vm_1 (i.e. reverse) */
    assert(g.edges[g.heads[vm_1] ^ 1].to       == vm_1);
    assert(g.edges[g.heads[vm_1] ^ 1].capacity == 1   );
    assert(g.edges[g.heads[vm_1] ^ 1].flow     == 0   );
    assert(g.edges[g.heads[vm_1] ^ 1].cost     == -1  );
    assert(g.edges[g.heads[vm_1] ^ 1].next     == -1  );

    /* pcpu_1 -> sink */
    assert(g.edges[g.heads[pcpu_1]].to       == sink               );
    assert(g.edges[g.heads[pcpu_1]].capacity == 0                  );
    assert(g.edges[g.heads[pcpu_1]].flow     == 1                  );
    assert(g.edges[g.heads[pcpu_1]].cost     == 0                  );
    assert(g.edges[g.heads[pcpu_1]].next     == (g.heads[vm_1] ^ 1));
    /* sink -> pcpu_1 (i.e. reverse)*/
    assert(g.edges[g.heads[pcpu_1] ^ 1].to       == pcpu_1);
    assert(g.edges[g.heads[pcpu_1] ^ 1].capacity == 1   );
    assert(g.edges[g.heads[pcpu_1] ^ 1].flow     == 0   );
    assert(g.edges[g.heads[pcpu_1] ^ 1].cost     == 0  );
    assert(g.edges[g.heads[pcpu_1] ^ 1].next     == -1  );

    printf("  PASS: test_mcmf_can_map_one_vm_to_one_cpu\n");
}

static void test_mcmf_can_map_two_vm_to_two_cpu_using_reverse_edge() {
    FlowGraph g;
    int nr_nodes = 6;
    graph_init(&g, nr_nodes);

    int source = 0;
    int vm_1 = 1;
    int vm_2 = 2;
    int pcpu_1 = 3;
    int pcpu_2 = 4;
    int sink = 5;
    int capacity = 1;
    
    graph_add_edge(&g, source, vm_1  , capacity, 0 );
    graph_add_edge(&g, vm_1  , pcpu_1, capacity, 1 );
    graph_add_edge(&g, pcpu_1, sink  , capacity, 0 );
    graph_add_edge(&g, source, vm_2  , capacity, 0 );
    graph_add_edge(&g, vm_2  , pcpu_2, capacity, 10);
    graph_add_edge(&g, pcpu_2, sink  , capacity, 0 );
    graph_add_edge(&g, vm_1  , pcpu_2, capacity, 3 );
    graph_add_edge(&g, vm_2  , pcpu_1, capacity, 5 );

    MCMFResult result = mcmf_solve(&g, source, sink);
    assert(result.total_cost == 8);
    assert(result.total_flow == 2);

    print_graph(&g);

    /* vm_1 -> pcpu_1 */
    assert(g.edges[g.edges[g.heads[vm_1]].next].to == pcpu_1 );
    assert(g.edges[g.edges[g.heads[vm_1]].next].capacity == 1);
    assert(g.edges[g.edges[g.heads[vm_1]].next].flow == 0    );
    /* pcpu_1 -> vm_1 (i.e. reverse) */
    assert(g.edges[g.edges[g.heads[vm_1]].next ^ 1].to == vm_1   );
    assert(g.edges[g.edges[g.heads[vm_1]].next ^ 1].capacity == 0);  // Reverse edge is used
    assert(g.edges[g.edges[g.heads[vm_1]].next ^ 1].flow == 1    );
    /* vm_1 -> pcpu_2 */
    assert(g.edges[g.heads[vm_1]].to == pcpu_2);
    assert(g.edges[g.heads[vm_1]].capacity == 0);
    assert(g.edges[g.heads[vm_1]].flow == 1);
    /* vm_2 -> pcpu_1 */
    assert(g.edges[g.heads[vm_2]].to == pcpu_1);
    assert(g.edges[g.heads[vm_2]].capacity == 0);
    assert(g.edges[g.heads[vm_2]].flow == 1);
    /* vm_2 -> pcpu_2 */
    assert(g.edges[g.edges[g.heads[vm_2]].next].to == pcpu_2 );
    assert(g.edges[g.edges[g.heads[vm_2]].next].capacity == 1);
    assert(g.edges[g.edges[g.heads[vm_2]].next].flow == 0    );

    printf("  PASS: test_mcmf_can_map_two_vm_to_two_cpu_using_reverse_edge\n");
}

int main(void) {
    printf("Running mcmf tests ...\n\n");

    test_graph_can_init();
    test_graph_can_add_edge();
    test_mcmf_can_map_one_vm_to_one_cpu();
    test_mcmf_can_map_two_vm_to_two_cpu_using_reverse_edge();

    printf("\nAll tests passed.\n");
    return 0;
}