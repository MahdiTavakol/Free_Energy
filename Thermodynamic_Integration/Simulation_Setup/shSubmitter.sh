#!/bin/bash

# Base job name
base_jobname="178TI"
# Array of increments
for i in {1..7}
do
    sbatch $base_jobname$i.slurm
    sleep 10
done


