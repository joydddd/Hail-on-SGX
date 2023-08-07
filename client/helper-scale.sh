#!/bin/bash
for _ in {1..10}
do
    echo "Log 5000"
    ./bin/client client_config5000.json >> scaling-5k-64-3.txt
    echo "Lin 5000"
    ./bin/client client_config5000.json >> scaling-5k-64-3.txt
    echo "Log 10000"
    ./bin/client client_config10000.json >> scaling-10k-64-3.txt
    echo "Lin 10000"
    ./bin/client client_config10000.json >> scaling-10k-64-3.txt
    echo "Log 20000"
    ./bin/client client_config20000.json >> scaling-20k-64-3.txt
    echo "Lin 20000"
    ./bin/client client_config20000.json>> scaling-20k-64-3.txt
done