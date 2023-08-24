#!/bin/bash
for _ in {1..5}
do
    echo "log-2 5000"
    ./gwashost compute_server_config1250log-3.json >> scaling-5k-64-3.txt
done

for _ in {1..5}
do
    echo "Lin 5000"
    ./gwashost compute_server_config1250lin-3.json >> scaling-5k-64-3.txt
done

for _ in {1..5}
do
    echo "log-2 10000"
    ./gwashost compute_server_config2500log-3.json >> scaling-10k-64-3.txt
done

for _ in {1..5}
do
     echo "Lin 10000"
    ./gwashost compute_server_config2500lin-3.json >> scaling-10k-64-3.txt
done

for _ in {1..5}
do
    echo "log-2 20000"
    ./gwashost compute_server_config5000log-3.json >> scaling-20k-64-3.txt
done

for _ in {1..5}
do
    echo "Lin 20000"
    ./gwashost compute_server_config5000lin-3.json >> scaling-20k-64-3.txt
done

for _ in {1..5}
do
    echo "Log 20000"
    ./gwashost compute_server_config5000log-famhe-3.json >> scaling-20k-64.txt
done

for _ in {1..5}
do
    echo "Lin 20000"
    ./gwashost compute_server_config5000lin-famhe-3.json >> scaling-20k-64.txt
done