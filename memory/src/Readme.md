# Memory Coordinator

The memory coordinator uses the Libvirt API to collect the host free memory inforation, and each VM's memory stats. These are stored in a `SystemState` for computing the new VM memory size. After adjustment is calcualted for each VM the coordinator then go ahead to use Libvirt API to set the new VM memory size.

## Data Structure

The system state stores the host free memory and a list of VMs.

```c
typedef struct {
    VM vms[MAX_VMS];
    int nr_vms;
    int free_memory;
} SystemState;
```

Each VM has a list of memory statistics that may be usefult for computing the new VM memory size.

```c
typedef struct {
    char name[MAX_NAME_LEN];  // VM's name (aka domain's name)
    int  id;
    int  max_memory;          // The maximum memory in KBytes allowed
    int  memory_unused;       // VIR_DOMAIN_MEMORY_STAT_UNUSED
    int  memory_available;    // VIR_DOMAIN_MEMORY_STAT_AVAILABLE
    int  memory_usable;       // VIR_DOMAIN_MEMORY_STAT_USABLE
    int  memory_rss;          // VIR_DOMAIN_MEMORY_STAT_RSS
    int  balloon_size;        // VIR_DOMAIN_MEMORY_STAT_ACTUAL_BALLOON
} VM;
```

## Compute New VM Memory Size

The `VIR_DOMAIN_MEMORY_STAT_AVAILABLE` is the indicator of how much memory can be taken away from the VM without impacting its performance.

This is according to the Libvirt API.

> `VIR_DOMAIN_MEMORY_STAT_AVAILABLE`: The total amount of usable memory as seen by the domain. This value may be less than the amount of memory assigned to the domain if a balloon driver is in use or if the guest OS does not initialize all assigned pages. This value is expressed in kiB.

The target available memory for each VM is 100 MB. The coordinator go through each VM and compare the available memory with this target.

With the Max adjustment can be made is 50 MB at a time, when adjustment absolute value is greater than 50 it's capped.

| Case | Available | Delta | Current | Target |
|---|---|---|---|---|
| Decrease | 125 | -25 | 512 | 487 |
| Decrease capped | 175 | -50 | 512 | 462 |
| Increase | 75 | +25 | 512 | 537 |
| Increase capped | 25 | +50 | 512 | 562 |

Delta is also cumulated and compared with the host's total available memory. If the addition to a VM requires host's free memory to drop below 200 MB this operation is skipped and the new target remains the same as the current size.

