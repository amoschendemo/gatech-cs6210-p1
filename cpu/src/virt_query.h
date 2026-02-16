#ifndef VIRT_QUERY_H
#define VIRT_QUERY_H

#include <libvirt.h>
#include "vm_types.h"

typedef struct {
    virConnectPtr conn;
} VirtContext;

int virt_query_state(VirtContext *ctx, SystemState *state);

int caculate_utilization_rate(SystemState *current, SystemState *previous);

int virt_apply_pinning(VirtContext *ctx, const SystemState *state, const Schedule *schedule);

#endif