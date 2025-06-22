#!/bin/bash

ARGS_NUM=$#

if [[ $ARGS_NUM -lt 1 ]]; then
  echo "Wrong number of arguments!"
  echo "Usage: bash $0 <NUMBER_OF_CLIENTS>"
  exit 1
fi

CLIENTS_NUM=$1
PIDS=()

# Preparation
for client in $(seq 1 $CLIENTS_NUM); do
  mosquitto_sub -t test &
  PIDS+=($!)
done

# Actual test
for client in $(seq 1 $CLIENTS_NUM); do
  mosquitto_pub -t test -m "Hello, world!"
done

for pid in ${PIDS[@]}; do
  kill $pid
done