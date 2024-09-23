#!/bin/bash

# Define the step size of the folder names
step=1000000

#loop from 1 to 50 (inclusive)
for ((i=1; i<=50; i++));
do
	folder_number=$((i*step))
	cp "template/SMD-reverse.in" "run-${folder_number}/SMD-reverse.in"
	cp "template/SMD.in" "run-${folder_number}/SMD.in"
done
