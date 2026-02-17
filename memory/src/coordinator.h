#include "vm_types.h"
#define TARGET_AVAILABLE_MB 200  // MB
#define ONE_K 1024
#define MAX_MEMORY_DELTA 50  // MB

int compute_vm_target_memory(SystemState *sys_state);
