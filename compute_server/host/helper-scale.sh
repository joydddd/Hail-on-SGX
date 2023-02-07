#!/bin/bash
# for snps in 125000 62500 #31250 15625 7813 3907 1954 977
# do
#     echo "SNPS" $snps
    for _ in {1..10}
    do
        # ./gwashost compute_server_config1000log.json
        # ./gwashost compute_server_config1000lin.json
        ./gwashost compute_server_config5000log.json
        # echo "Lin 5000"
        ./gwashost compute_server_config5000lin.json
        # echo "Log 10000"
        # ./gwashost compute_server_config10000log.json
        # echo "Lin 10000"
        # ./gwashost compute_server_config10000lin.json
        # echo "Lin 20000"
        # ./gwashost compute_server_config20000lin.json
    done
# done