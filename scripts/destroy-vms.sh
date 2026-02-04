#!/bin/bash

# Script to destroy all aos_vm* virtual machines
# Usage: ./destroy-vms.sh

echo "Destroying all aos_vm* virtual machines..."

# Get list of all aos_vm* VMs
VMS=$(virsh list --all --name | grep "^aos_vm")

if [ -z "$VMS" ]; then
    echo "No aos_vm* VMs found"
    exit 0
fi

echo "Found the following VMs to destroy:"
echo "$VMS"
echo ""

# Confirm before destroying
read -p "Are you sure you want to destroy these VMs? (yes/no): " confirm

if [ "$confirm" != "yes" ]; then
    echo "Cancelled"
    exit 0
fi

# Destroy each VM
for vm_name in $VMS; do
    echo "Destroying VM: $vm_name"
    uvt-kvm destroy "$vm_name"
    
    if [ $? -eq 0 ]; then
        echo "VM $vm_name destroyed successfully"
    else
        echo "Error destroying VM $vm_name"
    fi
done

echo "All VMs destroyed"
virsh list --all
