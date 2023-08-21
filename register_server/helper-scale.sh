#!/bin/bash
for _ in {1..5}
do
    echo "Log 5000"
    ./bin/register_server register_config.json >> scaling-5k-2:1.txt
    echo "Lin 5000"
    ./bin/register_server register_config.json >> scaling-5k-2:1.txt
    echo "Log 10000"
    ./bin/register_server register_config.json >> scaling-10k-2:1.txt
    echo "Lin 10000"
    ./bin/register_server register_config.json >> scaling-10k-2:1.txt
    echo "Log 20000"
    ./bin/register_server register_config.json >> scaling-20k-2:1.txt
    echo "Lin 20000"
    ./bin/register_server register_config.json >> scaling-20k-2:1.txt
done