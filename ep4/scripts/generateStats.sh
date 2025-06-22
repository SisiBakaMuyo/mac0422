#!/bin/bash

SCENARIOS=(0 100 1000)

make statsGenerator
for SCENARIO in ${SCENARIOS[@]}; do
  FOLDER=$(pwd)/results/$SCENARIO
  ./statsGenerator $FOLDER
done

make clean