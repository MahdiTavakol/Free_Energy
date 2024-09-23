#!/bin/bash

# Define the step size of the folder names
step=1000000

#loop from 1 to 50 (inclusive)
for ((i=1; i<=50; i++));
do
	folder_number=$((i*step))
	mkdir "run-${folder_number}"
	cp "0-InitialStructures(14-EQ-1-Series1)/EQ-Res-${folder_number}.dat" "run-${folder_number}/EQ-Res-${folder_number}.dat"
	cp "template/SMD-reverse.in" "run-${folder_number}/SMD-reverse.in"
	cp "template/SMD.in" "run-${folder_number}/SMD.in"
	cp "template/lmp-SMD-ARC" "run-${folder_number}/lmp-SMD-ARC"
	cp "template/lmp-SMD-SCARF" "run-${folder_number}/lmp-SMD-SCARF"	
done
