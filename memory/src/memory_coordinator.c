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

int is_exit = 0; // DO NOT MODIFY THE VARIABLE

void MemoryScheduler(virConnectPtr conn, int interval);

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

	signal(SIGINT, signal_callback_handler);

	while (!is_exit)
	{
		// Calls the MemoryScheduler function after every 'interval' seconds
		MemoryScheduler(conn, interval);
		sleep(interval);
	}

	// Close the connection
	virConnectClose(conn);
	return 0;
}

/*
COMPLETE THE IMPLEMENTATION
*/
void MemoryScheduler(virConnectPtr conn, int interval)
{
	// 1. Get host physical machine's hardware details
	virNodeInfo nodeinfo;
    if (virNodeGetInfo(conn, &nodeinfo) < 0) { // Get host CPU info
        return;
    }

	// 2. Get number of active VMs
	virDomainPtr *domains;
	unsigned int flags = VIR_CONNECT_LIST_DOMAINS_RUNNING |
						 VIR_CONNECT_LIST_DOMAINS_PERSISTENT;
	int nr_vms = virConnectListAllDomains(conn, &domains, flags);
	if (nr_vms < 0) {
		fprintf(stderr, "Failed to get list of domains\n");
		return;
	}
	
	for (int i = 0; i < nr_vms; i++) {
		virDomainPtr domain = domains[i];

		// 4.1 Get number of vCPUs for a given domain (i.e VM)
		virDomainInfo dominfo;
		if (virDomainGetInfo(domain, &dominfo) != 0) { // Get domain vCPU count
			fprintf(stderr, "Failed to get info for domain: %d\n", i);
			return;
		}

		printf("VM: %s\n", virDomainGetName(domain));
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
					printf("Actual: %llu KiB\n", stats[i].val); break;
				case VIR_DOMAIN_MEMORY_STAT_UNUSED:
					printf("Unused: %llu KiB\n", stats[i].val); break;
				case VIR_DOMAIN_MEMORY_STAT_AVAILABLE:
					printf("Available: %llu KiB\n", stats[i].val); break;
				case VIR_DOMAIN_MEMORY_STAT_USABLE:
					printf("Usable: %llu KiB\n", stats[i].val); break;
				case VIR_DOMAIN_MEMORY_STAT_RSS:
					printf("RSS: %llu KiB\n", stats[i].val); break;
				case VIR_DOMAIN_MEMORY_STAT_MAJOR_FAULT:
					printf("Major Faults: %llu\n", stats[i].val); break;
				case VIR_DOMAIN_MEMORY_STAT_MINOR_FAULT:
					printf("Minor Faults: %llu\n", stats[i].val); break;
				// Add other tags like SWAP_IN, SWAP_OUT, LAST_UPDATE as needed
			}
		}
		printf("--------------\n");
	}
}