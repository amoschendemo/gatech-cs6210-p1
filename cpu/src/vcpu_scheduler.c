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

void CPUScheduler(virConnectPtr conn, int interval);

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

/* COMPLETE THE IMPLEMENTATION */
void CPUScheduler(virConnectPtr conn, int interval)
{
	// List all VM's name and vCPU count and utilization.
	virNodeInfo nodeinfo;
	virDomainPtr *domains;
	const char *name;
	size_t i;
	int ret;
	int ncpus;
	virVcpuInfo *cpuinfo = NULL;
    unsigned char *cpumaps = NULL;
    virDomainInfo dominfo;
    int cpumaplen;
	int nhostcpus;

    if (virNodeGetInfo(conn, &nodeinfo) < 0) { // Get host CPU info
        return;
    }
    nhostcpus = VIR_NODEINFO_MAXCPUS(nodeinfo);
    cpumaplen = VIR_CPU_MAPLEN(nhostcpus); // Calculate map length in bytes

	unsigned int flags = VIR_CONNECT_LIST_DOMAINS_RUNNING |
						VIR_CONNECT_LIST_DOMAINS_PERSISTENT;
	ret = virConnectListAllDomains(conn, &domains, flags);
	if (ret < 0) {
		fprintf(stderr, "Failed to get list of domains\n");
		return;
	}

	for (i = 0; i < ret; i++) {
		if (virDomainGetInfo(domains[i], &dominfo) != 0) { // Get domain vCPU count
			return;
		}
		name = virDomainGetName(domains[i]);
		printf("Domain name: %s\n", name);
		
		// Allocate memory for info and cpumaps arrays
		cpuinfo = malloc(sizeof(virVcpuInfo) * dominfo.nrVirtCpu);
		cpumaps = malloc(dominfo.nrVirtCpu * cpumaplen);
		if (!cpuinfo || !cpumaps) {
			// Handle memory allocation failure
			free(cpuinfo);
			free(cpumaps);
			return;
		}

		ncpus = virDomainGetVcpus(domains[i], cpuinfo, dominfo.nrVirtCpu, cpumaps, cpumaplen);
		if (ncpus < 0) {
			// Handle error
			fprintf(stderr, "Error getting vcpu info\\n");
		} else {
			// Process the results in cpuinfo and cpumaps
			printf("Found %d vcpus\\n", ncpus);
			for (int i = 0; i < ncpus; i++) {
				printf("VCPU %d: state %d, cpu time %llu, real cpu %d\\n", cpuinfo[i].number, cpuinfo[i].state, cpuinfo[i].cpuTime, cpuinfo[i].cpu);
				// Further logic to interpret cpumaps (bitmap of pinned physical CPUs)
			}
		}

		virDomainFree(domains[i]);
	}
	free(domains);

	// Free allocated memory
	free(cpuinfo);
	free(cpumaps);
}