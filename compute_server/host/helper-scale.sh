#!/bin/bash
for _ in {1..5}
do
    echo "Log 5000"
    ./gwashost compute_server_config1250log.json >> scaling-5k-64.txt
done

for _ in {1..5}
do
    echo "Lin 5000"
    ./gwashost compute_server_config1250lin.json >> scaling-5k-64.txt
done

for _ in {1..5}
do
    echo "Log 10000"
    ./gwashost compute_server_config2500log.json >> scaling-10k-64.txt
done

for _ in {1..5}
do
     echo "Lin 10000"
    ./gwashost compute_server_config2500lin.json >> scaling-10k-64.txt
done

for _ in {1..5}
do
    echo "Log 20000"
    ./gwashost compute_server_config5000log.json >> scaling-20k-64.txt
done

for _ in {1..5}
do
    echo "Lin 20000"
    ./gwashost compute_server_config5000lin.json >> scaling-20k-64.txt
done