#!/bin/bash
for _ in {1..10}
do
    # ./gwashost compute_server_config1000log.json
    # ./gwashost compute_server_config1000lin.json
    # ./gwashost compute_server_config5000log.json
    # ./gwashost compute_server_config5000lin.json
    # ./gwashost compute_server_config10000log.json
    # ./gwashost compute_server_config10000lin.json
    echo "Lin 20000"
    ./gwashost compute_server_config20000lin.json
done