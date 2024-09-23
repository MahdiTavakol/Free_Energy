#!/bin/bash

for i in $(seq 1 25);
do
	echo "Submitting script_$i.sh"
	sbatch script_$i.sh
done
