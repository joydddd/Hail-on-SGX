#!/bin/bash

# for _ in {1..1}
# do
#     echo "Log 100000"
#     ./gwashost configs/compute_server_configlog-big4.json >> scaling-5k-64-4.txt
# done

for _ in {1..3}
do
    echo "Lin 100000"
    ./gwashost configs/compute_server_configlin-big4.json >> scaling-5k-64-4.txt
done

# for _ in {1..5}
# do
#     echo "Log 5000"
#     ./gwashost configs/compute_server_config1250log-4.json >> scaling-5k-64-4.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 5000"
#     ./gwashost configs/compute_server_config1250lin-4.json >> scaling-5k-64-4.txt
# done

# for _ in {1..5}
# do
#     echo "Log 10000"
#     ./gwashost configs/compute_server_config2500log-4.json >> scaling-10k-64-4.txt
# done

# for _ in {1..5}
# do
#      echo "Lin 10000"
#     ./gwashost configs/compute_server_config2500lin-4.json >> scaling-10k-64-4.txt
# done

# for _ in {1..5}
# do
#     echo "Log 20000"
#     ./gwashost configs/compute_server_config5000log-4.json >> scaling-20k-64-4.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 20000"
#     ./gwashost configs/compute_server_config5000lin-4.json >> scaling-20k-64-4.txt
# done

# for _ in {1..5}
# do
#     echo "Log 20000"
#     ./gwashost configs/compute_server_config5000log-famhe-4.json >> scaling-20k-64-4.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 20000"
#     ./gwashost configs/compute_server_config5000lin-famhe-4.json >> scaling-20k-64-4.txt
# done

# for _ in {1..5}
# do
#     echo "Log 20000"
#     ./gwashost configs/compute_server_config5000log-oblivious-4.json >> scaling-20k-64-4.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 20000"
#     ./gwashost configs/compute_server_config5000lin-oblivious-4.json >> scaling-20k-64-4.txt
# done
