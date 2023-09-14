#!/bin/bash
for _ in {1..15}
do
    echo "log-2 5000"
    ./gwashost configs/compute_server_config1250log-8.json >> scaling-5k-64-8.txt
done

for _ in {1..15}
do
    echo "Lin 5000"
    ./gwashost configs/compute_server_config1250lin-8.json >> scaling-5k-64-8.txt
done

for _ in {1..15}
do
    echo "log-2 10000"
    ./gwashost configs/compute_server_config2500log-8.json >> scaling-10k-64-8.txt
done

for _ in {1..15}
do
     echo "Lin 10000"
    ./gwashost configs/compute_server_config2500lin-8.json >> scaling-10k-64-8.txt
done

for _ in {1..15}
do
    echo "log-2 20000"
    ./gwashost configs/compute_server_config5000log-8.json >> scaling-20k-64-8.txt
done

for _ in {1..15}
do
    echo "Lin 20000"
    ./gwashost configs/compute_server_config5000lin-8.json >> scaling-20k-64-8.txt
done

for _ in {1..15}
do
    echo "Log 20000"
    ./gwashost configs/compute_server_config5000log-famhe-8.json >> scaling-20k-64.txt
done

for _ in {1..15}
do
    echo "Lin 20000"
    ./gwashost configs/compute_server_config5000lin-famhe-8.json >> scaling-20k-64.txt
done

for _ in {1..15}
do
    echo "Log 20000"
    ./gwashost configs/compute_server_config5000log-oblivious-8.json >> scaling-20k-64.txt
done

for _ in {1..15}
do
    echo "Lin 20000"
    ./gwashost configs/compute_server_config5000lin-oblivious-8.json >> scaling-20k-64.txt
done