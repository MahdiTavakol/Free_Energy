#!/bin/bash

 ./Jar_v5.02 $@ #Executalbe num_folders num_sims_per_folder(subfolders) num_data_per_sim folder_1/ folder_2/ .... folder_n/
 ./PlotJar.py
 ./WorkDistribution-v02.py $1 $2 $3
