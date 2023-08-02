#!/bin/bash
for _ in {1..10}
do
    echo "Log 5000"
    ./gwashost compute_server_config5000log.json >> scaling-5k-64.txt
    echo "Lin 5000"
    ./gwashost compute_server_config5000lin.json >> scaling-5k-64.txt
    echo "Log 10000"
    ./gwashost compute_server_config10000log.json >> scaling-10k-64.txt
    echo "Lin 10000"
    ./gwashost compute_server_config10000lin.json >> scaling-10k-64.txt
    echo "Log 20000"
    ./gwashost compute_server_config20000log.json >> scaling-20k-64.txt
    echo "Lin 20000"
    ./gwashost compute_server_config20000lin.json >> scaling-20k-64.txt
done