#!/bin/bash

# read n
# read x
# x=$(($2 * 100000))

for i in {1..5..1}
do
    for cov in $(eval echo {1..16..3})
    do 
        echo Covariants: $cov
        for count in $(eval echo {100000..2000000..100000})
        do
            echo Patients: $count
            echo $cov $count >> output-$cov.txt
            ./gwashost compute_server_config-$cov-$count.json >> output-$cov.txt
        done
    done
done