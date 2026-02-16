# VCPU Scheduler 

The scheduler uses Minimum Cost Maximum Flow (MCMF) as the mechanism for optimizing vCPU and pCPU assignment. The underlying algorithm is the Bellman-Ford variation - Shortest Path Fast Algorithm (SPFA) for calculating the cheapest paths. The scheduler periodically collects the current system state and creates a new graph for calculating the optimal vCPU and pCPU assignment.

# Data Structure

The scheduler uses three major data structure to support the algorithms and operations.

## System State

```c
typedef struct {
    VM vms[MAX_VMS];
    PCPU pcpus[MAX_PCPUS];
    int nr_vms;
    int nr_pcpus;
} SystemState;
```

A list of VMs and pCPUs are extracted through querying the `irConnectPtr conn`. For each VM we record its name, id, current pCPU, and usage. The usage is currently not used in the assignment calculation. It can be used as an enhancement.

```c
typedef struct {
    char               name[MAX_NAME_LEN];
    int                id;
    int                current_pcpu;
    double             cpu_usage_rate;
    unsigned long long cpu_time;
} VM;
```

The system state also stores the pCPU statistics. 

```c
typedef struct {
    int    id;
    double utilization_rate;
    unsigned long long idle_ns;
    double idle_rate;
} PCPU;
```

The utitlization rate collected through Libvirt remains zero during testing and have to use idle time to calculate the idle rate. The utilization rate used in creating the flow graph is derived from inverting the idle rate (i.e. 1 - idle rate).

## Flow Graph

```
       VM1 ----- pCPU1
      /     \ /       \
 Source     / \       Sink  
      \    /   \      /
       VM2 ----- pCPU2
```

A flow chart is used to represent all possible connections between VMs and pCPUs. An additional Source and Sink are added to use the Max Flow modeling.

The Flow Graph is a stored as an Adjacency List. Each forward edge comes with its reverse edge stored next to it.

```c
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
```

To look for edges comes out from node i first access `heads[i]` which gives the index to the last edge added for this node in the `edges`. Each edge also act as a linked list. The `next` field stores the index of the next edge share the same source as the current one.

```c
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
```

With following conditions to find all the VMs that connects from the source.

```c
int source = 0;

/* Get the last edge's index from heads */
int last_edge_index = heads[source];

Edge last_added_edge = edges[last_edge_index];

int second_to_the_last_edge_index = last_added_edge.next;

Edge second_to_the_last_edge = edges[second_to_the_last_edge_index];
```

### Reverse edge

When a forward edge (e.g. VM -> pCPU) is added a reverse edge is also added with a negative cost so that in the successive minimum cost path lookup it can be used by the algorithm to determine an alternative by reversing a previously selected forward path.

Switching between the pair of the forward and reverse edge is done through toggling the last bit in the index.

```c
Edge forward_edge = edges[i];
Edge reverse_edge = edges[i ^ 1];
```

## Schedule

A schedule is derived from the flow graph and it carries the VM/pCPU assignment information for sending to the `pin_vcpu_to_pcpu(...)` function to pin VM to pCPU.

```c
typedef struct {
    int vm_to_pcpu[MAX_VMS];   /* result: vm i assigned to pcpu vm_to_pcpu[i] */
    int num_assigned;
    int total_cost;
} Schedule;
```

# Algorithms

There are two major steps go through the graph that is created using the latest system state. First we calculate the cheapest paths that connects the Source and the Sink through all nodes. Once the paths are defined we then push through the capacity through the paths and the graph is then updated. Edges' capacity changed and some previous 

## 1. Find cheapest path

The following method uses SPFA to find the cheapest paths that connects source to all other nodes.

```c
static bool find_cheapest_path(FlowGraph *g, int source, int sink, int *out_dist, int *out_prev_edges);
```

Through this exercise each VM is bound to one pCPU. This is main driven by the nature of shortest path that enforces a pCPU gets assigned to one VM. The cheapest assignment wins and when two options has the same cost it will pick the first node in the option list. As a result as long as the nodes in the graph is added in the same order between iterations the assignment remains unchanged.


### SPFA

Instead of looping through all nodes and edges with O(|V| * |E|) with V as the number of vertices and E as the number of edges, SPFA uses a queue to store node has its distance just relaxed. SPFA looks more like Dijkstra algorithm but allow node's distance to be recaludated.

## 2. Update flow

Finding the cheapest path minimizes the cost and now we need to find the bottleneck of the flow. This is the step of maximuizing the flow. This steps updates the selected forward edges and its counterpart reverse edge in the flow chart with amount that can flow and the remaining capacity. This is necessary for another iteration of finding the cheapest path to discover alternative path selection that can furhter reduce the overall cost.

When all the forward capcacity is used up there will have no path to allow Source to connect with Sink. The optimal state is reached and a schedule can be built based on the resulted graph.

## 3. Extract assignment

Go through each VM node will find all the paths connects it to the possible pCPU options. The path that has a flow greater than 0 is the one that indicates the assignment. Update the `vm_to_pcpu` array by using SystemState's VM's index and set the pCPU id as the value in this array.

```c
typedef struct {
    int vm_to_pcpu[MAX_VMS];  // Each VM in SystemState has a matching space here.
    //..
} Schedule;
```

When comes to pin the VM to pCPU we go through the `SystemState.vms` to find VM's ID and then find its assigned pCPU's ID from `Schedule.vm_to_pcpu`.

## Citations

1. Minimum-cost flow, Algorithms for Competitive Programming, https://cp-algorithms.com/graph/min_cost_flow.html

2. C Program to Implement Adjacency List, GeeksforGeeks, https://www.geeksforgeeks.org/c/c-program-to-implement-adjacency-list/

3. Forward and Reverse Star Representation, Page 35 on https://www.princeton.edu/~alaink/Orf467F13/Readings&AssignmentNetworks.pdf

4. A Versatile Data Structure for Edge-oriented Qraph Algorlthms, https://dl.acm.org/doi/pdf/10.1145/214762.214769

