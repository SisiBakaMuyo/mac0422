#!/bin/bash

ARGS_NUM=$#

if [[ $ARGS_NUM -lt 4 ]]; then
  echo "Wrong number of arguments!"
  echo "Usage: bash $0 <CONTAINER_NAME> <NUMBER_OF_CLIENTS> <TEST_SCRIPT_PID> <INDEX>"
  exit 1
fi

CONTAINER_NAME=$1
TEST_SIZE=$2
TEST_PID=$3
INDEX=$4

TEST_FILENAME=results-$INDEX.txt
rm -f results/$TEST_SIZE/$TEST_FILENAME
touch results/$TEST_SIZE/$TEST_FILENAME

docker stats --no-stream --format "{{.CPUPerc}} | {{.NetIO}}" $CONTAINER_NAME >> results/$TEST_SIZE/$TEST_FILENAME

while ps -p $TEST_PID > /dev/null; do
  docker stats --no-stream --format "{{.CPUPerc}} | {{.NetIO}}" $CONTAINER_NAME >> results/$TEST_SIZE/$TEST_FILENAME
done