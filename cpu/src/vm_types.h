#ifndef VM_TYPES_H
#define VM_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_NAME_LEN 8
#define MAX_VMS      8
#define MAX_PCPUS    4

typedef struct {
    char          name[MAX_NAME_LEN];  // VM's name (aka domain's name)
    int           id;
    int           current_pcpu;
    double        cpu_usage;
    unsigned long last_cache_ts;
} VM;

typedef struct {
    int    id;
    double utilization;
} PCPU;

typedef struct {
    VM vms[MAX_VMS];
    PCPU pcpus[MAX_PCPUS];
    int nr_vms;
    int nr_pcpus;
} SystemState;

#endif