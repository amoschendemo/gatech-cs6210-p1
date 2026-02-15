# CPU Scheduler

Use Minimum Cost Flow modeling to schedule VMs to pCPUs.

## Model

For a graph that consist four sections. One Source node that connects all the VM nodes once with an edge. Then each VM node connects to all pCPU node once with an edge. In a four VMs and three pCPUs setup this section will require 3 x 4 = 12 edges. Then all the pCPU nodes are connected to a Sink node once with an edge.

Each edge will have a capacity assignment and a cost assignment. The algorithm is to calculate an assignment such that maximizes the capacity of the path that connects one VM to one pCPU and at the same time minimizes the cost.

### Source -> VM
Capacity: number of vCPU
Cost: Set to 0 for now. If VM utilization rate is also considered then (I'm not sure) set it to 3 when the rate is about 50% and 5 when above 75%.

### VM -> pCPU
Capacity: vCPU to pCPU ratio. Normally one vCPU is supported by one pCPU so it's 1. If two vCPUs are supported by one pCPU then it's 2, and if one vCPU is supported by two pCPU then it's 0.5.
Cost: Affinity is 0 cost and no affinity is 5. So preference is given to running VM on current assigned pCPU and there is a cost for moving VM from one to another pCPU.

### pCPU -> Sink
Capacity: If each pCPU normally handles one vCPU then it is 1. If one pCPU can handle two vCPUs then it is 2. In our situation we allow over booking and we set this to 2.
Cost: The cost is associated with the utilization rate. Before the utilization rate exceeds 50% the cost is set to 0 to encourage allocating new VM. When the rate reaches 50% the cost is set to 5. This is the same as the affinity bonus and the scheduler starts considering to move VM to other pCPU that has below 50% utilization rate.

### Reschedule delay
Delay: 1. This is a count gets decremented by one in each reschule cycle to avoid a VM being continuously shifted in each cycle. The VM is considered for reschedule when the count is 0. It gets reset back to the configured delay value (i.e. 1 for now) it reachs to 0.

## Data structure

```C
struct PcpuInfo {
  int index;
  unsigned long long cpu_time;
  double utilization_rate;
}

struct VcpuInfo {
  int index;
  unsigned long long cpu_time;
}

struct VmInfo {
  int index;
  char *name;
  struct VcpuInfo *vcpus;
  double utilization_rate;
}

struct ListNode {
  char *type;  # values: SOURCE | VM | PCPU | SINK
  struct PcpuInfo pcpu;
  struct VmInfo vm;
  struct GraphNode graph_node;
  struct ListNode next;
}

struct GraphNode {
  char *type;  # values: SOURCE | VM | PCPU | SINK
  int id;
  struct ListNode list_node;
}
```

## Algorithm

For a simplified implementation of Minimum Cost Maximum Flow (MCMF) I choose Successive Shortest Path algorithm with SPFA (Shortest Path Faster Algoirthm).
