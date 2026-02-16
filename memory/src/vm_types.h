#ifndef VM_TYPES_H
#define VM_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_NAME_LEN 8
#define MAX_VMS      8
#define MAX_PCPUS    4

typedef struct {
    char name[MAX_NAME_LEN];  // VM's name (aka domain's name)
    int  id;
    bool set_stats_period;
    int  max_memory;          // The maximum memory in KBytes allowed
    int  memory_unused;       // VIR_DOMAIN_MEMORY_STAT_UNUSED
    int  memory_available;    // VIR_DOMAIN_MEMORY_STAT_AVAILABLE
    int  memory_usable;       // VIR_DOMAIN_MEMORY_STAT_USABLE
    int  memory_rss;          // VIR_DOMAIN_MEMORY_STAT_RSS
    int  balloon_size;        // VIR_DOMAIN_MEMORY_STAT_ACTUAL_BALLOON
} VM;

typedef struct {
    VM vms[MAX_VMS];
    int nr_vms;
    unsigned long long free_memory_bytes;
} SystemState;

#endif