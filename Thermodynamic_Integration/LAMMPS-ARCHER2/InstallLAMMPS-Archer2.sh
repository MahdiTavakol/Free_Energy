#!/bin/bash
module load cpe/22.12
module load cray-fftw/3.3.10.3
module load cmake/3.21.3
module load eigen/3.4.0
module load cray-python

export LD_LIBRARY_PATH=$CRAY_LD_LIBRARY_PATH:$LD_LIBRARY_PATH

rm -rf build-archer2
mkdir build-archer2
cd build-archer2
cmake -C ../cmake/presets/most.cmake -D BUILD_MPI=yes -DCMAKE_BUILD_TYPE=RELEASE ../cmake
cmake --build . --parallel 
