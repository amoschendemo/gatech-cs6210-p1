#ifndef VM_TYPES_H
#define VM_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_NAME_LEN 8
#define MAX_VMS      8

/**
 * @brief System information supports the problem domain
 * 
 * @param memory_usable_kb How much the balloon can be inflated without pushing the guest system to swap, corresponds to 'Available' in /proc/meminfo
 */
typedef struct {
    char name[MAX_NAME_LEN];  // VM's name (aka domain's name)
    int  id;
    int  max_memory_kb;          // The maximum memory in KBytes allowed
    int  memory_unused_kb;       // VIR_DOMAIN_MEMORY_STAT_UNUSED
    int  memory_available_kb;    // VIR_DOMAIN_MEMORY_STAT_AVAILABLE
    int  memory_usable_kb;       // VIR_DOMAIN_MEMORY_STAT_USABLE
    int  memory_rss_kb;          // VIR_DOMAIN_MEMORY_STAT_RSS
    int  balloon_size_kb;        // VIR_DOMAIN_MEMORY_STAT_ACTUAL_BALLOON
    int  target_memory_kb;       // Used for setting new VM memory size
} VM;

typedef struct {
    VM vms[MAX_VMS];
    int nr_vms;
    unsigned long long free_memory_bytes;
} SystemState;

#endif