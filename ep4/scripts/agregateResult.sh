#!/bin/bash

SCENARIOS=(0 100 1000)
N=100

make agregateStats
for SCENARIO in ${SCENARIOS[@]}; do
  for I in $(seq 1 $N); do
    FOLDER=$(pwd)/results/$SCENARIO
    ./agregateStats $FOLDER results-$I.txt
  done
done

make clean
