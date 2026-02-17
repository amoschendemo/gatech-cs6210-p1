#include <math.h>
#include <stdlib.h>
#include "vm_types.h"
#include "coordinator.h"

static int min(int a, int b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

/**
 * @brief Update VM's target memory for setting new memory size
 * 
 * @return -1 in error or number of VMs updated
 */
int update_vm_target_memory(SystemState *sys_state) {
    int vms_updated = 0;
	for (int i = 0; i < sys_state->nr_vms; i++) {
		VM *vm = &sys_state->vms[i];
		int delta = abs(vm->memory_available_kb - TARGET_AVAILABLE_MB * ONE_K);
        delta = min(delta, MAX_MEMORY_DELTA * ONE_K);
        if (vm->memory_available_kb < TARGET_AVAILABLE_MB * ONE_K) {
            vm->target_memory_kb = vm->balloon_size_kb + delta;
        } else {
            vm->target_memory_kb = vm->balloon_size_kb - delta;
        }
        vms_updated++;
	}
    return vms_updated;
}