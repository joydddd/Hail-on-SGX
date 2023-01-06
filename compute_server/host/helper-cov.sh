#!/bin/bash
for cov in {1..16}
do 
    echo Covariants: $cov
    for count in {100000..2000000..100000}
    do
        echo Patients: $count
        ./gwashost compute_server_config-$cov-$count.json
    done
done