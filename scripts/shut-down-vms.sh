#!/bin/bash

# Script to shut down all aos_vm* virtual machines
# Usage: ./shut-down-vms.sh

echo "Shutting down all aos_vm* virtual machines..."

# Get list of all aos_vm* VMs
VMS=$(virsh list --all --name | grep "^aos_vm")

if [ -z "$VMS" ]; then
    echo "No aos_vm* VMs found"
    exit 0
fi

echo "Found the following VMs to shut down:"
echo "$VMS"
echo ""

# Confirm before shutting down
read -p "Are you sure you want to shut down these VMs? (yes/no): " confirm

if [ "$confirm" != "yes" ]; then
    echo "Cancelled"
    exit 0
fi

# Shut down each VM
for vm_name in $VMS; do
    echo "Shutting down VM: $vm_name"
    virsh shutdown "$vm_name"

    if [ $? -eq 0 ]; then
        echo "VM $vm_name is shut down successfully"
    else
        echo "Error shutting down VM $vm_name"
    fi
done

echo "All VMs shut down"
virsh list --all
