#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "vm_types.h"
#include "scheduler.h"
#include "virt_query.h"

static int get_number_of_pcpus(VirtContext *ctx) {
    virNodeInfo nodeinfo;
    if (virNodeGetInfo(ctx->conn, &nodeinfo) < 0) {
        return -1;
    }
    return nodeinfo.cpus;
}

int virt_query_state(VirtContext *ctx, SystemState *state) {
    /* Reset system state */
    memset(state, 0, sizeof(SystemState));

    /* Number of PCPUs */
    int nr_pcpus = get_number_of_pcpus(ctx);
    if ( nr_pcpus < 0) {
        fprintf(stderr, "Failed to get node information\n");
        return -1;
    }
    state->nr_pcpus = nr_pcpus;
    
    /* PCPU usage */
    for(int i = 0; i < nr_pcpus; i++){
        state->pcpus[i].id = i;
        // First call with nr_stats=0 to get the number of supported stats for this CPU
        virNodeCPUStats params[VIR_NODE_CPU_STATS_FIELD_LENGTH];
        int nr_stats = 0;
        if (virNodeGetCPUStats(ctx->conn, i, NULL, &nr_stats, 0) == 0 && nr_stats != 0) {
            if (virNodeGetCPUStats(ctx->conn, i, params, &nr_stats, 0) == 0) {
                for (int j = 0; j < nr_stats; j++) {
                    // Search specifically for the 'idle' field
                    if (strcmp(params[j].field, VIR_NODE_CPU_STATS_UTILIZATION) == 0) {
                        state->pcpus[i].utilization_rate = params[j].value / 100.0;
                        break;
                    }
                }
            }
        }
    }

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

        /* Get number of vCPUs for a given domain (i.e VM) */
		virDomainInfo dominfo;
		if (virDomainGetInfo(domain, &dominfo) != 0) { // Get domain vCPU count
			fprintf(stderr, "Failed to get info for domain: %d\n", i);
			return -1;
		}
		int nr_vcpus = dominfo.nrVirtCpu;

        if (nr_vcpus != 1) {
            fprintf(stderr, "Only support VM with 1 vCPU but found %d vCPUs\n", nr_vcpus);
            return -1;
        }

		/* Allocate memory for getting virtual CPU info */
        virVcpuInfoPtr vcpuinfos = malloc(nr_vcpus * sizeof(virVcpuInfo));
        size_t pcpu_maplen = VIR_CPU_MAPLEN(nr_pcpus);
        unsigned char *cpumaps  = malloc(nr_vcpus * pcpu_maplen);
        int ret = virDomainGetVcpus(domain, vcpuinfos, nr_vcpus, cpumaps, pcpu_maplen);
        if (ret < 0) {
            // Handle error
            fprintf(stderr, "Error getting vcpu info\n");
            return -1;
        } else if (ret == 1) {
            state->vms[i].current_pcpu = vcpuinfos[0].cpu;
            state->vms[i].cpu_time = vcpuinfos[0].cpuTime;
        } else {
            // Handle error
            fprintf(stderr, "Error getting more than one vcpu info\n");
            return -1;
        }
		
		virDomainFree(domains[i]);
    }
    return 0;
}

int caculate_utilization_rate(SystemState *current, SystemState *previous, unsigned long long interval_ns) {
    /* calculate current VM's utilization rate */
    int vms_updated = 0;
    for (int i = 0; i < current->nr_vms; i++) {
        for (int j = 0; j < previous->nr_vms; j++) {
            /* find matching previous vm */
            if (current->vms[i].name == previous->vms[j].name) {
                current->vms[i].cpu_usage_rate = (current->vms[i].cpu_time - previous->vms[j].cpu_time) * 100.0 / interval_ns;
                vms_updated++;
                break;
            }
        }
    }
    return vms_updated;
}

int virt_apply_pinning(VirtContext *ctx, const SystemState *state, const Schedule *schedule) {
    return -1;
}