#!/bin/bash
# for _ in {1..5}
# do
#     echo "Log 5000"
#     ./bin/register_server register_config.json >> scaling-5k-1024-2.txt
# done

for _ in {1..5}
do
    echo "Lin 5000"
    ./bin/register_server register_config.json >> scaling-5k-1024-2.txt
done

for _ in {1..5}
do
    echo "Log 10000"
    ./bin/register_server register_config.json >> scaling-10k-1024-2.txt
done

for _ in {1..5}
do
    echo "Lin 10000"
    ./bin/register_server register_config.json >> scaling-10k-1024-2.txt
done

for _ in {1..5}
do
    echo "Log 20000"
    ./bin/register_server register_config.json >> scaling-20k-1024-2.txt
done

for _ in {1..5}
do
    echo "Lin 20000"
    ./bin/register_server register_config.json >> scaling-20k-1024-2.txt
done

for _ in {1..5}
do
    echo "Log 20000"
    ./bin/register_server register_config.json >> scaling-20k-famhe-1024-2.txt
done

for _ in {1..5}
do
    echo "Lin 20000"
    ./bin/register_server register_config.json >> scaling-20k-famhe-1024-2.txt
done