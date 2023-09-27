#!/bin/bash
for _ in {1..5}
do
    echo "log-2 5000"
    ./gwashost configs/compute_server_config1250log-7.json >> scaling-5k-64-7.txt
done

for _ in {1..5}
do
    echo "Lin 5000"
    ./gwashost configs/compute_server_config1250lin-7.json >> scaling-5k-64-7.txt
done

for _ in {1..5}
do
    echo "log-2 10000"
    ./gwashost configs/compute_server_config2500log-7.json >> scaling-10k-64-7.txt
done

for _ in {1..5}
do
     echo "Lin 10000"
    ./gwashost configs/compute_server_config2500lin-7.json >> scaling-10k-64-7.txt
done

for _ in {1..5}
do
    echo "log-2 20000"
    ./gwashost configs/compute_server_config5000log-7.json >> scaling-20k-64-7.txt
done

for _ in {1..5}
do
    echo "Lin 20000"
    ./gwashost configs/compute_server_config5000lin-7.json >> scaling-20k-64-7.txt
done

for _ in {1..5}
do
    echo "Log 20000"
    ./gwashost configs/compute_server_config5000log-famhe-7.json >> scaling-20k-64.txt
done

for _ in {1..5}
do
    echo "Lin 20000"
    ./gwashost configs/compute_server_config5000lin-famhe-7.json >> scaling-20k-64.txt
done

for _ in {1..5}
do
    echo "Log 20000"
    ./gwashost configs/compute_server_config5000log-oblivious-7.json >> scaling-20k-64.txt
done

for _ in {1..5}
do
    echo "Lin 20000"
    ./gwashost configs/compute_server_config5000lin-oblivious-7.json >> scaling-20k-64.txt
done