#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "coordinator.h"

static void test_coordinator_can_increase_less_than_max_delta() {
    SystemState sys_state;
    memset(&sys_state, 0, sizeof(SystemState));

    sys_state.nr_vms = 1;
    sys_state.free_memory_bytes = 12L * ONE_K * ONE_K * ONE_K;  // 12 GB
    sys_state.vms[0].memory_available_kb = 175 * ONE_K;         // 175 MB
    sys_state.vms[0].balloon_size_kb = 512 * ONE_K;             // 512 MB

    int vm_updated = update_vm_target_memory(&sys_state);

    assert(vm_updated == 1);
    assert(sys_state.vms[0].target_memory_kb == 537 * ONE_K);

    printf("PASS test_coordinator_can_increase_less_than_max_delta\n");
}

static void test_coordinator_can_increase_up_to_max_delta() {
    SystemState sys_state;
    memset(&sys_state, 0, sizeof(SystemState));

    sys_state.nr_vms = 1;
    sys_state.free_memory_bytes = 12L * ONE_K * ONE_K * ONE_K;  // 12 GB
    sys_state.vms[0].memory_available_kb = 125 * ONE_K;         // 125 MB
    sys_state.vms[0].balloon_size_kb = 512 * ONE_K;             // 512 MB

    int vm_updated = update_vm_target_memory(&sys_state);

    assert(vm_updated == 1);
    assert(sys_state.vms[0].target_memory_kb == 562 * ONE_K);

    printf("PASS test_coordinator_can_increase_up_to_max_delta\n");
}

static void test_coordinator_can_decrease_less_than_max_delta() {
    SystemState sys_state;
    memset(&sys_state, 0, sizeof(SystemState));

    sys_state.nr_vms = 1;
    sys_state.free_memory_bytes = 12L * ONE_K * ONE_K * ONE_K;  // 12 GB
    sys_state.vms[0].memory_available_kb = 225 * ONE_K;         // 125 MB
    sys_state.vms[0].balloon_size_kb = 512 * ONE_K;             // 512 MB

    int vm_updated = update_vm_target_memory(&sys_state);

    assert(vm_updated == 1);
    assert(sys_state.vms[0].target_memory_kb == 487 * ONE_K);

    printf("PASS test_coordinator_can_decrease_less_than_max_delta\n");
}

static void test_coordinator_can_decrease_up_to_max_delta() {
    SystemState sys_state;
    memset(&sys_state, 0, sizeof(SystemState));

    sys_state.nr_vms = 1;
    sys_state.free_memory_bytes = 12L * ONE_K * ONE_K * ONE_K;  // 12 GB
    sys_state.vms[0].memory_available_kb = 275 * ONE_K;         // 125 MB
    sys_state.vms[0].balloon_size_kb = 512 * ONE_K;             // 512 MB

    int vm_updated = update_vm_target_memory(&sys_state);

    assert(vm_updated == 1);
    assert(sys_state.vms[0].target_memory_kb == 462 * ONE_K);

    printf("PASS test_coordinator_can_decrease_up_to_max_delta\n");
}

int main(void) {
    printf("Running coordinator tests ...\n\n");

    test_coordinator_can_increase_less_than_max_delta();
    test_coordinator_can_increase_up_to_max_delta();
    test_coordinator_can_decrease_less_than_max_delta();
    test_coordinator_can_decrease_up_to_max_delta();

    printf("\nAll tests passed.\n");
    return 0;
}