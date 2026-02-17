#include "vm_types.h"
#define TARGET_VM_AVAILABLE_MB 100
#define TARGET_HOST_FREE_MB 200 
#define ONE_K 1024
#define MAX_MEMORY_DELTA_MB 50

int compute_vm_target_memory(SystemState *sys_state);
