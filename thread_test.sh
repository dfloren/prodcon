#!/bin/bash

counter=0
while [ $? -eq 0 ]; do
	./prodcon 1 1 < inputexample3
    echo "Iteration $counter, Command: './prodcon 1 1 < inputexample3'"
    counter=$((counter+1))
done
