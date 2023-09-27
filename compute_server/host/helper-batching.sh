#!/bin/bash

for VAR in {1..5..1}
do
    echo "1"
    ./gwashost_1 compute_server_config10000log.json >> batch-1.txt
    ./gwashost_1 compute_server_config10000lin.json >> batch-1.txt

    echo "2"
    ./gwashost_2 compute_server_config10000log.json >> batch-2.txt
    ./gwashost_2 compute_server_config10000lin.json >> batch-2.txt

    echo "4"
    ./gwashost_4 compute_server_config10000log.json >> batch-4.txt
    ./gwashost_4 compute_server_config10000lin.json >> batch-4.txt

    echo "8"
    ./gwashost_8 compute_server_config10000log.json >> batch-8.txt
    ./gwashost_8 compute_server_config10000lin.json >> batch-8.txt

    echo "16"
    ./gwashost_16 compute_server_config10000log.json >> batch-16.txt
    ./gwashost_16 compute_server_config10000lin.json >> batch-16.txt

    echo "32"
    ./gwashost_32 compute_server_config10000log.json >> batch-32.txt
    ./gwashost_32 compute_server_config10000lin.json >> batch-32.txt

    echo "64"
    ./gwashost_64 compute_server_config10000log.json >> batch-64.txt
    ./gwashost_64 compute_server_config10000lin.json >> batch-64.txt

    echo "128"
    ./gwashost_128 compute_server_config10000log.json >> batch-128.txt
    ./gwashost_128 compute_server_config10000lin.json >> batch-128.txt

    echo "256"
    ./gwashost_256 compute_server_config10000log.json >> batch-256.txt
    ./gwashost_256 compute_server_config10000lin.json >> batch-256.txt

    echo "512"
    ./gwashost_512 compute_server_config10000log.json >> batch-512.txt
    ./gwashost_512 compute_server_config10000lin.json >> batch-512.txt

    echo "1024"
    ./gwashost_1024 compute_server_config10000log.json >> batch-1024.txt
    ./gwashost_1024 compute_server_config10000lin.json >> batch-1024.txt
    
done