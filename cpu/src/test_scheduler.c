#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "mcmf.h"
#include "scheduler.h"

static void test_can_four_vms_four_pcpus_with_zero_utilitzation() {
    SystemState state;
    memset(&state, -1, sizeof(SystemState));
    /* Setup PCPUs  */
    state.nr_pcpus = 4;
    state.pcpus[0].id = 0;
    state.pcpus[0].utilization_rate = 0.0;
    state.pcpus[1].id = 1;
    state.pcpus[1].utilization_rate = 0.0;
    state.pcpus[2].id = 2;
    state.pcpus[2].utilization_rate = 0.0;
    state.pcpus[3].id = 3;
    state.pcpus[3].utilization_rate = 0.0;
    /* Setup VMs */
    state.nr_vms = 4;
    state.vms[0].id = 0;
    snprintf(state.vms[0].name, MAX_NAME_LEN, "%s", "aos_vm1");
    state.vms[0].current_pcpu = 0;
    state.vms[0].cpu_time = 0;
    state.vms[0].cpu_usage_rate = 0.0;
    state.vms[1].id = 0;
    snprintf(state.vms[1].name, MAX_NAME_LEN, "%s", "aos_vm2");
    state.vms[1].current_pcpu = 1;
    state.vms[1].cpu_time = 0;
    state.vms[1].cpu_usage_rate = 0.0;
    state.vms[2].id = 0;
    snprintf(state.vms[2].name, MAX_NAME_LEN, "%s", "aos_vm3");
    state.vms[2].current_pcpu = 2;
    state.vms[2].cpu_time = 0;
    state.vms[2].cpu_usage_rate = 0.0;
    state.vms[3].id = 0;
    snprintf(state.vms[3].name, MAX_NAME_LEN, "%s", "aos_vm4");
    state.vms[3].current_pcpu = 3;
    state.vms[3].cpu_time = 0;
    state.vms[3].cpu_usage_rate = 0.0;

    Schedule schedule = compute_schedule(&state);

    assert(schedule.num_assigned == 4);
    assert(schedule.total_cost == 0);
    assert(schedule.vm_to_pcpu[0] == 0);
    assert(schedule.vm_to_pcpu[1] == 1);
    assert(schedule.vm_to_pcpu[2] == 2);
    assert(schedule.vm_to_pcpu[3] == 3);
    
    printf("PASS test_can_four_vms_four_pcpus_with_zero_utilitzation\n");
}

static void test_can_four_vms_four_pcpus_with_25p_utilitzation() {
    SystemState state;
    memset(&state, -1, sizeof(SystemState));
    /* Setup PCPUs  */
    state.nr_pcpus = 4;
    state.pcpus[0].id = 0;
    state.pcpus[0].utilization_rate = 25.0;
    state.pcpus[1].id = 1;
    state.pcpus[1].utilization_rate = 0.0;
    state.pcpus[2].id = 2;
    state.pcpus[2].utilization_rate = 0.0;
    state.pcpus[3].id = 3;
    state.pcpus[3].utilization_rate = 0.0;
    /* Setup VMs */
    state.nr_vms = 4;
    state.vms[0].id = 0;
    snprintf(state.vms[0].name, MAX_NAME_LEN, "%s", "aos_vm1");
    state.vms[0].current_pcpu = 0;
    state.vms[0].cpu_time = 0;
    state.vms[0].cpu_usage_rate = 0.0;
    state.vms[1].id = 0;
    snprintf(state.vms[1].name, MAX_NAME_LEN, "%s", "aos_vm2");
    state.vms[1].current_pcpu = 1;
    state.vms[1].cpu_time = 0;
    state.vms[1].cpu_usage_rate = 0.0;
    state.vms[2].id = 0;
    snprintf(state.vms[2].name, MAX_NAME_LEN, "%s", "aos_vm3");
    state.vms[2].current_pcpu = 2;
    state.vms[2].cpu_time = 0;
    state.vms[2].cpu_usage_rate = 0.0;
    state.vms[3].id = 0;
    snprintf(state.vms[3].name, MAX_NAME_LEN, "%s", "aos_vm4");
    state.vms[3].current_pcpu = 3;
    state.vms[3].cpu_time = 0;
    state.vms[3].cpu_usage_rate = 0.0;

    Schedule schedule = compute_schedule(&state);

    print_schedule(&schedule, 4);

    assert(schedule.num_assigned == 4);
    assert(schedule.total_cost == 25);
    assert(schedule.vm_to_pcpu[0] == 0);
    assert(schedule.vm_to_pcpu[1] == 1);
    assert(schedule.vm_to_pcpu[2] == 2);
    assert(schedule.vm_to_pcpu[3] == 3);
    
    printf("PASS test_can_four_vms_four_pcpus_with_25p_utilitzation\n");
}

static void test_can_four_vms_four_pcpus_with_75p_utilitzation() {
    SystemState state;
    memset(&state, -1, sizeof(SystemState));
    /* Setup PCPUs  */
    state.nr_pcpus = 4;
    state.pcpus[0].id = 0;
    state.pcpus[0].utilization_rate = 75.0;
    state.pcpus[1].id = 1;
    state.pcpus[1].utilization_rate = 0.0;
    state.pcpus[2].id = 2;
    state.pcpus[2].utilization_rate = 0.0;
    state.pcpus[3].id = 3;
    state.pcpus[3].utilization_rate = 0.0;
    /* Setup VMs */
    state.nr_vms = 4;
    state.vms[0].id = 0;
    snprintf(state.vms[0].name, MAX_NAME_LEN, "%s", "aos_vm1");
    state.vms[0].current_pcpu = 0;
    state.vms[0].cpu_time = 0;
    state.vms[0].cpu_usage_rate = 0.0;
    state.vms[1].id = 0;
    snprintf(state.vms[1].name, MAX_NAME_LEN, "%s", "aos_vm2");
    state.vms[1].current_pcpu = 1;
    state.vms[1].cpu_time = 0;
    state.vms[1].cpu_usage_rate = 0.0;
    state.vms[2].id = 0;
    snprintf(state.vms[2].name, MAX_NAME_LEN, "%s", "aos_vm3");
    state.vms[2].current_pcpu = 2;
    state.vms[2].cpu_time = 0;
    state.vms[2].cpu_usage_rate = 0.0;
    state.vms[3].id = 0;
    snprintf(state.vms[3].name, MAX_NAME_LEN, "%s", "aos_vm4");
    state.vms[3].current_pcpu = 3;
    state.vms[3].cpu_time = 0;
    state.vms[3].cpu_usage_rate = 0.0;

    Schedule schedule = compute_schedule(&state);

    print_schedule(&schedule, 4);

    assert(schedule.num_assigned == 4);
    assert(schedule.total_cost == 50);
    assert(schedule.vm_to_pcpu[0] == 3);  // Pick the first edge that is cheaper. (PCPU 3 is the edge added the last so it appears first)
    assert(schedule.vm_to_pcpu[1] == 1);
    assert(schedule.vm_to_pcpu[2] == 2);
    assert(schedule.vm_to_pcpu[3] == 3);
    
    printf("PASS test_can_four_vms_four_pcpus_with_75p_utilitzation\n");
}

int main(void) {
    printf("Running scheduler tests ...\n\n");

    test_can_four_vms_four_pcpus_with_zero_utilitzation();
    test_can_four_vms_four_pcpus_with_25p_utilitzation();
    test_can_four_vms_four_pcpus_with_75p_utilitzation();

    printf("\nAll tests passed.\n");
    return 0;
}
