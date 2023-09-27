#!/bin/bash

for _ in {1..5}
do
    echo "Log 5000"
    ./bin/register_server register_config.json >> scaling-5k-256-LAN.txt
done

for _ in {1..5}
do
    echo "Lin 5000"
    ./bin/register_server register_config.json >> scaling-5k-256-LAN.txt
done

for _ in {1..5}
do
    echo "Log 10000"
    ./bin/register_server register_config.json >> scaling-10k-256-LAN.txt
done

for _ in {1..5}
do
    echo "Lin 10000"
    ./bin/register_server register_config.json >> scaling-10k-256-LAN.txt
done

for _ in {1..5}
do
    echo "Log 20000"
    ./bin/register_server register_config.json >> scaling-20k-256-LAN.txt
done

for _ in {1..5}
do
    echo "Lin 20000"
    ./bin/register_server register_config.json >> scaling-20k-256-LAN.txt
done

# for _ in {1..5}
# do
#     echo "Log 20000"
#     ./bin/register_server register_config.json >> scaling-20k-famhe-256-LAN.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 20000"
#     ./bin/register_server register_config.json >> scaling-20k-famhe-256-LAN.txt
# done

# for _ in {1..5}
# do
#     echo "Log 20000"
#     ./bin/register_server register_config.json >> scaling-20k-oblivious-256-LAN.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 20000"
#     ./bin/register_server register_config.json >> scaling-20k-oblivious-256-LAN.txt
# done