#!/bin/bash
for cov in {16..16}
do 
    echo Covariants: $cov
    for count in {100000..1500000..100000}
    do
        ./gwashost compute_server_config-$cov-$count.json
    done
done