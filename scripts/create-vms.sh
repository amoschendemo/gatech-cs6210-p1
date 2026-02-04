#!/bin/bash

# Script to create multiple VMs using uvt-kvm
# Usage: ./createvms.sh <number>
# Where <number> is between 1 and 10

if [ $# -ne 1 ]; then
    echo "Usage: $0 <number>"
    echo "Error: Please provide a number between 1 and 10"
    exit 1
fi

NUM_VMS=$1

# Validate input is a number between 1 and 10
if ! [[ $NUM_VMS =~ ^[0-9]+$ ]] || [ $NUM_VMS -lt 1 ] || [ $NUM_VMS -gt 10 ]; then
    echo "Error: Argument must be a number between 1 and 10"
    exit 1
fi

echo "Creating $NUM_VMS virtual machine(s)..."

# Create VMs with names aos_vm1, aos_vm2, etc.
for i in $(seq 1 $NUM_VMS); do
    VM_NAME="aos_vm$i"
    echo "Creating VM: $VM_NAME"
    uvt-kvm create "$VM_NAME" release=bionic --memory=512
    
    if [ $? -eq 0 ]; then
        echo "Waiting for VM $VM_NAME to boot..."
        uvt-kvm wait "$VM_NAME"
        echo "VM $VM_NAME is ready"
    else
        echo "Error creating VM $VM_NAME"
        exit 1
    fi
done

echo "Successfully created $NUM_VMS VM(s)"
virsh list --all
