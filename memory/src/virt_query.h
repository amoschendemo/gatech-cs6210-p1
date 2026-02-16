#ifndef VIRT_QUERY_H
#define VIRT_QUERY_H

#include <libvirt/libvirt.h>
#include "vm_types.h"

typedef struct {
    virConnectPtr conn;
} VirtContext;

int set_vm_memory_stats(VirtContext *ctx);

int virt_query_state(VirtContext *ctx, SystemState *state);

void print_sys_state(SystemState *state);

#endif