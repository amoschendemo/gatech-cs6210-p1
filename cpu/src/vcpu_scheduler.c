#include <stdio.h>
#include <stdlib.h>
#include <libvirt/libvirt.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#define MIN(a, b) ((a) < (b) ? a : b)
#define MAX(a, b) ((a) > (b) ? a : b)

int is_exit = 0; // DO NOT MODIFY THIS VARIABLE

struct VcpuInfo {
	int index;
	int pcpu_index;
	double utilization;
};

struct VmInfo {
	const char *name;
	virDomainPtr domain;
	int nr_vcpus;
	struct VcpuInfo *vcpus;
};

void CPUScheduler(virConnectPtr conn, int interval);
int get_active_vms(struct VmInfo **out_vms, virConnectPtr conn, virNodeInfo nodeinfo, int interval);
int get_vcpus(struct VcpuInfo **out_vcpus, virDomainPtr domain, int nr_vcpus, virNodeInfo nodeinfo, int interval);
int pin_vcpu_to_pcpu(virDomainPtr domain, int vcpu_index, int cpumaplen, int pcpu_index);

/*
DO NOT CHANGE THE FOLLOWING FUNCTION
*/
void signal_callback_handler()
{
	printf("Caught Signal");
	is_exit = 1;
}

/*
DO NOT CHANGE THE FOLLOWING FUNCTION
*/
int main(int argc, char *argv[])
{
	virConnectPtr conn;

	if (argc != 2)
	{
		printf("Incorrect number of arguments\n");
		return 0;
	}

	// Gets the interval passes as a command line argument and sets it as the STATS_PERIOD for collection of balloon memory statistics of the domains
	int interval = atoi(argv[1]);

	conn = virConnectOpen("qemu:///system");
	if (conn == NULL)
	{
		fprintf(stderr, "Failed to open connection\n");
		return 1;
	}

	// Get the total number of pCpus in the host
	signal(SIGINT, signal_callback_handler);

	while (!is_exit)
	// Run the CpuScheduler function that checks the CPU Usage and sets the pin at an interval of "interval" seconds
	{
		CPUScheduler(conn, interval);
		sleep(interval);
	}

	// Closing the connection
	virConnectClose(conn);
	return 0;
}

// The last saved cumulative idle CPU time from previous interval
unsigned long long *previous_pcpu_stats_idles = NULL;

/* COMPLETE THE IMPLEMENTATION */
void CPUScheduler(virConnectPtr conn, int interval)
{
	// 1. Get host physical machine's hardware details
	virNodeInfo nodeinfo;
    if (virNodeGetInfo(conn, &nodeinfo) < 0) { // Get host CPU info
        return;
    }

	unsigned long long interval_ns = interval * 1000000000L;
	// 2. Get host CPU info
	int nr_pcpus = nodeinfo.cpus;
	if (previous_pcpu_stats_idles == NULL) {
		previous_pcpu_stats_idles = calloc(nr_pcpus, sizeof(double));
	}
	double *pcpu_stats_idle_rates = calloc(nr_pcpus, sizeof(double));
	for (int i = 0; i < nr_pcpus; i++) {
        virNodeCPUStats params[VIR_NODE_CPU_STATS_FIELD_LENGTH];
        int nr_stats = 0;

        // First call with nr_stats=0 to get the number of supported stats for this CPU
        if (virNodeGetCPUStats(conn, i, NULL, &nr_stats, 0) == 0 && nr_stats != 0) {
            if (virNodeGetCPUStats(conn, i, params, &nr_stats, 0) == 0) {
                for (int j = 0; j < nr_stats; j++) {
                    // Search specifically for the 'idle' field
                    if (strcmp(params[j].field, VIR_NODE_CPU_STATS_IDLE) == 0) {
                        printf("pCPU %d Idle Time: %llu ns\n", i, params[j].value);
						unsigned long long idle_ns = params[j].value;
						if (previous_pcpu_stats_idles[i] > 0) {
							pcpu_stats_idle_rates[i] = (idle_ns - previous_pcpu_stats_idles[i]) / interval_ns;
						}
						previous_pcpu_stats_idles[i] = params[j].value;
                    }
                }
            }
        }
	}
	for (int i = 0; i < nr_pcpus; i++) {
		printf("pCPU %d Idle Rate: %f%%\n", i, pcpu_stats_idle_rates[i]);
	}

	// 3. Get active VMs
	struct VmInfo *vms = NULL;
	int nr_vms = get_active_vms(&vms, conn, nodeinfo, interval);
	if (nr_vms < 0) {
		fprintf(stderr, "Failed to get list of active VMs\n");
		return;
	}
	printf("Found %d active VMs\n", nr_vms);
	for (int i = 0; i < nr_vms; i++) {
		struct VmInfo vm = vms[i];
		printf("%s, vCPUs [",vm.name);
		int nr_vcpus  = vm.nr_vcpus;
		for (int j = 0; j < nr_vcpus; j++) {
			struct VcpuInfo vcpu = vm.vcpus[j];
			printf("{index: %d, pcpu: %d, utilization %f%%}", vcpu.index, vcpu.pcpu_index, vcpu.utilization);
			if (j + 1 < nr_vcpus) {
				printf(", ");
			}
		}
		printf("]\n");
	}

	if (previous_pcpu_stats_idles) {
		free(previous_pcpu_stats_idles);
	}
	free(vms);
}

/**
 * @brief Get a list of active VMs
 * 
 * Allocates and initializes an array of VmInfo. Use virConnectListAllDomains,
 * virDomainGetInfo, and virDomainGetVcpus to extract VM's vCPU info.
 * 
 * @param conn: An active connection to a hypervisor.
 * @param out_vms: A pointer to the pointer that will hold the new address.
 * @return: The number of VmInfos allocated on success, or -1 on error.
 */
int get_active_vms(struct VmInfo **out_vms, virConnectPtr conn, virNodeInfo nodeinfo, int interval){
	// 1. Safety check
    if (out_vms == NULL) {
        return -1; 
    }

	// 2. Get number of active VMs
	virDomainPtr *domains;
	unsigned int flags = VIR_CONNECT_LIST_DOMAINS_RUNNING |
						 VIR_CONNECT_LIST_DOMAINS_PERSISTENT;
	int nr_vms = virConnectListAllDomains(conn, &domains, flags);
	if (nr_vms < 0) {
		fprintf(stderr, "Failed to get list of domains\n");
		return -1;
	}

	// 3. Allocate memory on the HEAP
    *out_vms = malloc(nr_vms * sizeof(struct VmInfo));

	// 4. Get Virtual Machine's name and vCPU usage
	for (int i = 0; i < nr_vms; i++) {
		virDomainPtr domain = domains[i];

		// 4.1 Get number of vCPUs for a given domain (i.e VM)
		virDomainInfo dominfo;
		if (virDomainGetInfo(domain, &dominfo) != 0) { // Get domain vCPU count
			fprintf(stderr, "Failed to get info for domain: %d\n", i);
			return -1;
		}

		// 4.2 Allocate memory for getting virtual CPU info
		int nr_vcpus = dominfo.nrVirtCpu;
		struct VcpuInfo *vcpuinfos = malloc(nr_vcpus * sizeof(struct VcpuInfo));
		if (get_vcpus(&vcpuinfos, domain, nr_vcpus, nodeinfo, interval) != nr_vcpus) {
			fprintf(stderr, "Failed to get virtual CPU info for domain: %d\n", i);
			return -1;
		}
		
		struct VmInfo vm = {
			.name = virDomainGetName(domain),
			.domain = domain,
			.nr_vcpus = nr_vcpus,
			.vcpus = vcpuinfos
		};
		(*out_vms)[i] = vm;
		
		virDomainFree(domains[i]);
		free(vcpuinfos);
	}

	free(domains);
	return nr_vms;
}


/**
 * @brief Get virtual CPU information
 * 
 * @param out_vcpus: list of VcpuInfo
 * @return -1 when memory allocation fails, 0 otherwise.
 */
int get_vcpus(struct VcpuInfo **out_vcpus, virDomainPtr domain, int nr_vcpus, virNodeInfo nodeinfo, int interval) {
	// 1. Get physical CPUs mapping length in bytes. This is used later when getting vcpu info.
	int nr_pcpus = nodeinfo.cpus;
	size_t pcpu_maplen = VIR_CPU_MAPLEN(nr_pcpus);

	// 2. Memory allocation for vcpuinfo and cpumaps
    virVcpuInfoPtr vcpuinfos = malloc(nr_vcpus * sizeof(virVcpuInfo));
    unsigned char *cpumaps  = malloc(nr_vcpus * pcpu_maplen);

	// 3. Get vCPU info
	int ret = virDomainGetVcpus(domain, vcpuinfos, nr_vcpus, cpumaps, pcpu_maplen);
	if (ret < 0) {
		// Handle error
		fprintf(stderr, "Error getting vcpu info\n");
	} else {
		// Process the results in cpuinfo and cpumaps
		printf("Found %d vcpus\n", ret);
		for (int i = 0; i < ret; i++) {
			virVcpuInfo vcpu_info = vcpuinfos[i];
			struct VcpuInfo vcpu = {
				.index = vcpu_info.number,
				.pcpu_index = vcpu_info.cpu,
				.utilization = vcpu_info.cpuTime / interval,
			};
			(*out_vcpus)[i] = vcpu;
		}
	}

	free(vcpuinfos);
	free(cpumaps);
	return ret;
}

struct PcpuInfo {
	int index;
	double utilization;
};

/**
 * @brief Pin a virtual CPU to a physical CPU.
 *
 * This function pins a virtual CPU (vCPU) of a domain to a specific 
 * physical CPU (pCPU) on the host. It allocates a CPU map (bitmask) 
 * to specify which pCPU the vCPU should be pinned to, and then uses 
 * the libvirt API to set the CPU affinity for the vCPU.
 *
 * @param domain The domain pointer.
 * @param vcpu_index The virtual CPU index.
 * @param cpumaplen The length of the CPU map buffer in bytes.
 * @param pcpu_index The physical CPU index to pin the virtual CPU to.
 * @return -1 when memory allocation fails, 0 otherwise.
 */
int pin_vcpu_to_pcpu(virDomainPtr domain, int vcpu_index, int cpumaplen, int pcpu_index) {
	// 1. Allocate the bitmask (cpumap) for the number of host CPUs (nhostcpus) using calloc to initialize it to all zeros
	unsigned char *cpumap;
	cpumap = calloc(1, cpumaplen); // Initialize map to all zeros
	if (!cpumap) {
		fprintf(stderr, "Memory allocation failed for cpumap\n");
        return -1;
    }

	// 2. Set the bit for the physical CPU (pcpu_index) in the cpumap.
    VIR_USE_CPU(cpumap, pcpu_index);

	// 3. Apply the pinning to the domain
    if (virDomainPinVcpu(domain, vcpu_index, cpumap, cpumaplen) < 0) {
        fprintf(stderr, "Failed to pin vCPU\n");
        free(cpumap);
        return -1;
    }

	printf("Successfully pinned vCPU %d to pCPU %d\n", vcpu_index, pcpu_index);

	free(cpumap);
	return 0;
}