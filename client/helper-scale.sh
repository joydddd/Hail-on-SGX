#!/bin/bash
for _ in {1..10}
do
    # echo "Log 1000"
    # ./bin/client client_config1000.json
    # echo "Lin 1000"
    # ./bin/client client_config1000.json
    # echo "Log 5000"
    # ./bin/client client_config5000.json
    # echo "Lin 5000"
    # ./bin/client client_config5000.json
    # echo "Log 10000"
    # ./bin/client client_config10000.json
    # echo "Lin 10000"
    # ./bin/client client_config10000.json
    echo "Lin 20000"
    ./bin/client client_config20000.json
done