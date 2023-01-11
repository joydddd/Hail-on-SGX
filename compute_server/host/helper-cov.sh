#!/bin/bash

# read n
# read x
x=$(($2 * 100000))

for cov in $(eval echo {$1..16..3})
do 
    echo Covariants: $cov
    for count in $(eval echo {$x..2000000..100000})
    do
        echo Patients: $count
        ./gwashost compute_server_config-$cov-$count.json >> output-$cov-3.txt
    done
done