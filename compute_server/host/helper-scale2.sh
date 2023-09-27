#!/bin/bash

for _ in {1..5}
do
    echo "Log 5000"
    ./gwashost configs/compute_server_config1250log-2.json >> scaling-5k-64-2.txt
done

for _ in {1..5}
do
    echo "Lin 5000"
    ./gwashost configs/compute_server_config1250lin-2.json >> scaling-5k-64-2.txt
done

for _ in {1..5}
do
    echo "Log 10000"
    ./gwashost configs/compute_server_config2500log-2.json >> scaling-10k-64-2.txt
done

for _ in {1..5}
do
     echo "Lin 10000"
    ./gwashost configs/compute_server_config2500lin-2.json >> scaling-10k-64-2.txt
done

for _ in {1..5}
do
    echo "Log 20000"
    ./gwashost configs/compute_server_config5000log-2.json >> scaling-20k-64-2.txt
done

for _ in {1..5}
do
    echo "Lin 20000"
    ./gwashost configs/compute_server_config5000lin-2.json >> scaling-20k-64-2.txt
done

# for _ in {1..5}
# do
#     echo "Log 20000"
#     ./gwashost configs/compute_server_config5000log-famhe-2.json >> scaling-20k-64-2.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 20000"
#     ./gwashost configs/compute_server_config5000lin-famhe-2.json >> scaling-20k-64-2.txt
# done

# for _ in {1..5}
# do
#     echo "Log 20000"
#     ./gwashost configs/compute_server_config5000log-oblivious-2.json >> scaling-20k-64-2.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 20000"
#     ./gwashost configs/compute_server_config5000lin-oblivious-2.json >> scaling-20k-64-2.txt
# done
