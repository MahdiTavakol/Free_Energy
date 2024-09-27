#!/bin/bash
numSeries=3
step=30000000


Sims=()
Coeffs=()

# Fill arrays
for i in $(seq 0 0.02 1.00)
    do
    # Format the number with two decimal places
    formatted=$(printf "%.2f" $i)
    Coeffs+=($formatted)

    # Replace '.' with '-' for the second array
    formatted_with_hyphen=${formatted/./-}
    Sims+=($formatted_with_hyphen)
done

for index in "${!Sims[@]}"
do
    mkdir "${Sims[$index]}"
    cd "${Sims[$index]}"
    for i in $(seq 1 $numSeries)
    do
        mkdir "$i-Series$i"
        cd "$i-Series$i"
        cp ../../EQmodify.py ./EQmodify.py
        cp ../../../1-EQ-\(8-HAp-001-pH5-size0-0.1MNaCl\)/$((i+1))-Series$((i+1))-4-EQ-dump/EQ-Res-${step}.dat ./EQ-Res-${step}.dat
        chmod +x ./EQmodify.py
        ./EQmodify.py "${Coeffs[$index]}" ./EQ-Res-${step}.dat ./EQ-Res-${step}-${Coeffs[$index]}.dat
        cp ../../TI.in ./TI.in
        scale=${Coeffs[$index]}
        sed -i "s/xXx/${scale}/g" TI.in
        cd ../
    done
    cd ../
done

echo "done!"

