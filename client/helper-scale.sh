#!/bin/bash

for _ in {1..5}
do
    echo "Lin 20000"
    ./bin/client client_config5000.json >> client-omega-20k-famhe-512.txt
done

for _ in {1..5}
do
    echo "Lin 20000"
    ./bin/client client_config5000.json >> client-omega-20k-oblivious-512.txt
done

for _ in {1..5}
do
    echo "Log 5000"
    ./bin/client client_config1250.json >> client-omega-5k-512.txt
done

for _ in {1..5}
do
    echo "Lin 5000"
    ./bin/client client_config1250.json >> client-omega-5k-512.txt
done

for _ in {1..5}
do
    echo "Log 10000"
    ./bin/client client_config2500.json >> client-omega-10k-512.txt
done

for _ in {1..5}
do
    echo "Lin 10000"
    ./bin/client client_config2500.json >> client-omega-10k-512.txt
done

for _ in {1..5}
do
    echo "Log 20000"
    ./bin/client client_config5000.json >> client-omega-20k-512.txt
done

for _ in {1..5}
do
    echo "Lin 20000"
    ./bin/client client_config5000.json >> client-omega-20k-512.txt
done

for _ in {1..5}
do
    echo "Log 20000"
    ./bin/client client_config5000.json >> client-omega-20k-famhe-512.txt
done

for _ in {1..5}
do
    echo "Log 20000"
    ./bin/client client_config5000.json >> client-omega-20k-oblivious-512.txt
done
