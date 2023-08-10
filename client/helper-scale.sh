#!/bin/bash
for _ in {1..5}
do
    echo "Log 5000"
    ./bin/client client_config1250.json >> scaling-5k-64-32.txt
    echo "Lin 5000"
    ./bin/client client_config1250.json >> scaling-5k-64-32.txt
    echo "Log 10000"
    ./bin/client client_config2500.json >> scaling-10k-64-32.txt
    echo "Lin 10000"
    ./bin/client client_config2500.json >> scaling-10k-64-32.txt
    echo "Log 20000"
    ./bin/client client_config5000.json >> scaling-20k-64-32.txt
    echo "Lin 20000"
    ./bin/client client_config5000.json>> scaling-20k-64-32.txt
done