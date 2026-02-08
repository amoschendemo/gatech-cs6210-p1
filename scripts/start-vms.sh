#!/bin/bash

# Script to start all aos_vm* virtual machines
# Usage: ./start-vms.sh

echo "Starting all aos_vm* virtual machines..."

# Get list of all aos_vm* VMs
VMS=$(virsh list --all --name | grep "^aos_vm")

if [ -z "$VMS" ]; then
    echo "No aos_vm* VMs found"
    exit 0
fi

echo "Found the following VMs to start:"
echo "$VMS"
echo ""

# Confirm before starting
read -p "Are you sure you want to start these VMs? (yes/no): " confirm

if [ "$confirm" != "yes" ]; then
    echo "Cancelled"
    exit 0
fi

# Start each VM
for vm_name in $VMS; do
    echo "Starting VM: $vm_name"
    virsh start "$vm_name"

    if [ $? -eq 0 ]; then
        echo "VM $vm_name is started successfully"
    else
        echo "Error starting VM $vm_name"
    fi
done

echo "All VMs started"
virsh list --all
