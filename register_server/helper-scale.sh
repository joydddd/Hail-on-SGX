#!/bin/bash

for _ in {1..3}
do
    echo "Log 5000"
    ./bin/register_server register_config.json >> scaling-5k-64-new.txt
done

for _ in {1..3}
do
    echo "Lin 5000"
    ./bin/register_server register_config.json >> scaling-5k-64-new.txt
done

for _ in {1..3}
do
    echo "Log 10000"
    ./bin/register_server register_config.json >> scaling-10k-64-new.txt
done

for _ in {1..3}
do
    echo "Lin 10000"
    ./bin/register_server register_config.json >> scaling-10k-64-new.txt
done

for _ in {1..3}
do
    echo "Log 20000"
    ./bin/register_server register_config.json >> scaling-20k-64-new.txt
done

for _ in {1..3}
do
    echo "Lin 20000"
    ./bin/register_server register_config.json >> scaling-20k-64-new.txt
done

# for _ in {1..3}
# do
#     echo "Log 20000"
#     ./bin/register_server register_config.json >> scaling-20k-famhe-64-new.txt
# done

# for _ in {1..3}
# do
#     echo "Log 20000"
#     ./bin/register_server register_config.json >> scaling-20k-oblivious-64-new.txt
# done