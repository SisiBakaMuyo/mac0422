#!/bin/bash

SCENARIOS=(0 100 1000)
N=100

mkdir -p results
cd results

for SIZE in ${SCENARIOS[@]}; do
  TEST_PATH=$SIZE
  rm -rf $TEST_PATH
  mkdir $TEST_PATH
done

cd ..
for INDEX in $(seq 1 $N); do
  for CLIENTS in ${SCENARIOS[@]}; do
    docker-compose up -d
    CONTAINER_ID=$(docker ps -q)
    sleep 0.5;

    bash runClients.sh $CLIENTS &
    PID=$!

    bash collectMeasurements.sh $CONTAINER_ID $CLIENTS $PID $INDEX

    docker-compose down
  done
done