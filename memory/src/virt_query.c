#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "vm_types.h"
#include "virt_query.h"

int virt_query_state(VirtContext *ctx, SystemState *state) {
    /* Reset system state */
    memset(state, 0, sizeof(SystemState));

    /* Host free memory */


    /* Number of VMs */
    virDomainPtr *domains;
	unsigned int flags = VIR_CONNECT_LIST_DOMAINS_RUNNING |
						 VIR_CONNECT_LIST_DOMAINS_PERSISTENT;
	int nr_vms = virConnectListAllDomains(ctx->conn, &domains, flags);
	if (nr_vms < 0) {
		fprintf(stderr, "Failed to get list of domains\n");
		return -1;
	}
    state->nr_vms = nr_vms;

    /* VM's CPU time */
    for (int i = 0; i < nr_vms; i++) {
        /* Get Virtual Machine's name and vCPU usage */
		virDomainPtr domain = domains[i];
        const char *vm_name = virDomainGetName(domain);
        if (vm_name) {
            snprintf(state->vms[i].name, MAX_NAME_LEN, "%s", vm_name);
        }
        state->vms[i].id = virDomainGetID(domain);

        /* Get physical RAM limit the VM was booted with */
        virDomainInfo dominfo;
		if (virDomainGetInfo(domain, &dominfo) != 0) { // Get domain vCPU count
			fprintf(stderr, "Failed to get info for domain: %d\n", i);
			return -1;
		}
		state->vms[i].max_memory = dominfo.maxMem;

        /* Get VM's memory stats */
		virDomainMemoryStatStruct stats[VIR_DOMAIN_MEMORY_STAT_NR];
		int nr_stats;
		// Retrieve stats. The flags parameter (last) is currently unused (0).
		nr_stats = virDomainMemoryStats(domain, stats, VIR_DOMAIN_MEMORY_STAT_NR, 0);
		if (nr_stats < 0) {
			fprintf(stderr, "Failed to get memory statistics\n");
			return;
		}

		for (int i = 0; i < nr_stats; i++) {
			switch (stats[i].tag) {
				case VIR_DOMAIN_MEMORY_STAT_ACTUAL_BALLOON:
                    /*
                    The current amount of RAM allocated to the guest by the hypervisor. 
                    This is the "visible" limit. If the balloon is inflated, this value decreases.
                    */
					state->vms[i].balloon_size = stats[i].val; break;
				case VIR_DOMAIN_MEMORY_STAT_UNUSED:
                    /*
                    The amount of RAM that is strictly empty/free. It is not being used for processes 
                    or even for disk caching.
                    */
					state->vms[i].memory_unused = stats[i].val; break;
				case VIR_DOMAIN_MEMORY_STAT_AVAILABLE:
                    /*
                    The total amount of RAM the Guest OS thinks it has (as reported by the guest kernel). 
                    This is usually ACTUAL_BALLOON minus some reserved kernel overhead.
                    */
					state->vms[i].memory_available = stats[i].val; break;
				case VIR_DOMAIN_MEMORY_STAT_USABLE:
                    /*
                    A more "realistic" look at free memory. It includes UNUSED plus memory used for caches/buffers 
                    that the OS could easily reclaim if a process asked for it.
                    */
					state->vms[i].memory_usable = stats[i].val; break;
				case VIR_DOMAIN_MEMORY_STAT_RSS:
                    /*
                    Resident Set Size. This is the physical memory on the Host machine currently consumed 
                    by the VM process.
                    */
					state->vms[i].memory_rss = stats[i].val; break;
			}
		}
		virDomainFree(domains[i]);
    }
    return 0;
}

void print_sys_state(SystemState *state) {
    printf("System state\n");
    printf("------------\n");
    printf("Available: %d", state->free_memory);
    printf("------------\n");
	for(int i = 0; i < state->nr_vms; i++){
		printf(
			"%d: VM %d (%s) max: %d, unused: %d, available: %d, balloon: %d\n",
			i,
            state->vms[i].id,
			state->vms[i].name,
            state->vms[i].max_memory,
			state->vms[i].memory_unused,
			state->vms[i].memory_available,
            state->vms[i].balloon_size
		);
	}
}