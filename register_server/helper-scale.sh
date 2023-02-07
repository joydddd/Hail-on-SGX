#!/bin/bash
# for snps in 125000 62500 #31250 15625 7813 3907 1954 977
# do
#     echo "SNPS" $snps
    for _ in {1..10}
    do
        # echo "Log 1000"
        # ./bin/register_server register_config.json
        # echo "Lin 1000"
        # ./bin/register_server register_config.json
        echo "Log 5000"
        ./bin/register_server register_config.json
        echo "Lin 5000"
        ./bin/register_server register_config.json
        # echo "Log 10000"
        # ./bin/register_server register_config.json
        # echo "Lin 10000"
        # ./bin/register_server register_config.json

        # echo "Lin 20000"
        # ./bin/register_server register_config.json
    done
# done