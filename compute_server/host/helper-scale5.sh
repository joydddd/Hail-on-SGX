#!/bin/bash
for _ in {1..3}
do
    echo "log-2 5000"
    ./gwashost configs/compute_server_config1250log-5.json >> scaling-5k-64-5.txt
done

for _ in {1..3}
do
    echo "Lin 5000"
    ./gwashost configs/compute_server_config1250lin-5.json >> scaling-5k-64-5.txt
done

for _ in {1..3}
do
    echo "log-2 10000"
    ./gwashost configs/compute_server_config2500log-5.json >> scaling-10k-64-5.txt
done

for _ in {1..3}
do
     echo "Lin 10000"
    ./gwashost configs/compute_server_config2500lin-5.json >> scaling-10k-64-5.txt
done

for _ in {1..3}
do
    echo "log-2 20000"
    ./gwashost configs/compute_server_config5000log-5.json >> scaling-20k-64-5.txt
done

for _ in {1..3}
do
    echo "Lin 20000"
    ./gwashost configs/compute_server_config5000lin-5.json >> scaling-20k-64-5.txt
done

for _ in {1..3}
do
    echo "Log 20000"
    ./gwashost configs/compute_server_config5000log-famhe-5.json >> scaling-20k-64.txt
done

for _ in {1..3}
do
    echo "Lin 20000"
    ./gwashost configs/compute_server_config5000lin-famhe-5.json >> scaling-20k-64.txt
done

for _ in {1..3}
do
    echo "Log 20000"
    ./gwashost configs/compute_server_config5000log-oblivious-5.json >> scaling-20k-64.txt
done

for _ in {1..3}
do
    echo "Lin 20000"
    ./gwashost configs/compute_server_config5000lin-oblivious-5.json >> scaling-20k-64.txt
done