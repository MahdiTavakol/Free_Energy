#!/bin/bash

 ./Jar_v5.05 $@ #3 30 1501 1-Series1/ 2-Series2/ 3-Series3/
 ./PlotJar-Ins.py
 ./WorkDistribution-v02.py $1 $2 $3
