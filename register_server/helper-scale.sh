#!/bin/bash
for _ in {1..10}
do
    echo "Log 1000"
    ./bin/register_server register_config.json
    # echo "Lin 1000"
    # ./bin/register_server register_config.json
    echo "Log 5000"
    ./bin/register_server register_config.json
    # echo "Lin 5000"
    # ./bin/register_server register_config.json
    echo "Log 10000"
    ./bin/register_server register_config.json
    # echo "Lin 10000"
    # ./bin/register_server register_config.json
done