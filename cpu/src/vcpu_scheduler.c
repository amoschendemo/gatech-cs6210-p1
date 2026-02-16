#include <stdio.h>
#include <stdlib.h>
#include <libvirt/libvirt.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include "virt_query.h"
#include "vm_types.h"
#include "scheduler.h"
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

// The last saved cumulative idle CPU time from previous interval
SystemState previous_sys_state;

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
	VirtContext ctx = {
		.conn = conn
	};
	SystemState current_sys_state;
	if(virt_query_state(ctx, &current_sys_state) < 0) {
		fprintf(stderr, "Failed to query the current system state\n");
	}
	printf("Found %d VMs, %d pCPUs\n", current_sys_state.nr_vms, current_sys_state.nr_pcpus);
	
	if (previous_sys_state == NULL) {
		printf("Skip the cycle for gathering more system information for scheduling\n")
		previous_sys_state = current_sys_state;
		return;
	}
	caculate_utilization_rate(&current_sys_state, &previous_sys_state);
	
	printf("System state\n");
	for(int i = 0; i < current_sys_state.nr_vms; i++){
		printf(
			"VM %d (%s) pCPU: %d, utilization: %.2f%%\n",
			current_sys_state.vms[i].id,
			current_sys_state.vms[i].name,
			current_sys_state.vms[i].current_pcpu,
			current_sys_state.vms[i].cpu_usage_rate
		);
	}
	for(int i = 0; i < current_sys_state.nr_pcpus; i++){
		printf(
			"PCPU %d utilization: %.2f%%\n",
			current_sys_state.pcpus[i].id,
			current_sys_state.pcpus[i].utilization_rate
		);
	}

	Schedule schedule = compute_schedule(&current_sys_state);
	printf("\nSchedule\n");
	for(int i = 0; i < current_sys_state.nr_vms; i++) {
		printf("VM %d -> PCPU %d", i, schedule.vm_to_pcpu[i])
	}
}

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