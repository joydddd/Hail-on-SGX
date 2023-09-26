#!/bin/bash
for _ in {1..3}
do
    echo "log-2 5000"
    ./gwashost configs/compute_server_config1250log-6.json >> scaling-5k-64-6.txt
done

for _ in {1..3}
do
    echo "Lin 5000"
    ./gwashost configs/compute_server_config1250lin-6.json >> scaling-5k-64-6.txt
done

for _ in {1..3}
do
    echo "log-2 10000"
    ./gwashost configs/compute_server_config2500log-6.json >> scaling-10k-64-6.txt
done

for _ in {1..3}
do
     echo "Lin 10000"
    ./gwashost configs/compute_server_config2500lin-6.json >> scaling-10k-64-6.txt
done

for _ in {1..3}
do
    echo "log-2 20000"
    ./gwashost configs/compute_server_config5000log-6.json >> scaling-20k-64-6.txt
done

for _ in {1..3}
do
    echo "Lin 20000"
    ./gwashost configs/compute_server_config5000lin-6.json >> scaling-20k-64-6.txt
done

for _ in {1..3}
do
    echo "Log 20000"
    ./gwashost configs/compute_server_config5000log-famhe-6.json >> scaling-20k-64.txt
done

for _ in {1..3}
do
    echo "Lin 20000"
    ./gwashost configs/compute_server_config5000lin-famhe-6.json >> scaling-20k-64.txt
done

for _ in {1..3}
do
    echo "Log 20000"
    ./gwashost configs/compute_server_config5000log-oblivious-6.json >> scaling-20k-64.txt
done

for _ in {1..3}
do
    echo "Lin 20000"
    ./gwashost configs/compute_server_config5000lin-oblivious-6.json >> scaling-20k-64.txt
done