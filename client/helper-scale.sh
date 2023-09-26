#!/bin/bash

for _ in {1..3}
do
    echo "Log 5000"
    ./bin/client client_config1250.json >> client-omega-5k-64-new.txt
done

for _ in {1..3}
do
    echo "Lin 5000"
    ./bin/client client_config1250.json >> client-omega-5k-64-new.txt
done

for _ in {1..3}
do
    echo "Log 10000"
    ./bin/client client_config2500.json >> client-omega-10k-64-new.txt
done

for _ in {1..3}
do
    echo "Lin 10000"
    ./bin/client client_config2500.json >> client-omega-10k-64-new.txt
done

for _ in {1..3}
do
    echo "Log 20000"
    ./bin/client client_config5000.json >> client-omega-20k-64-new.txt
done

for _ in {1..3}
do
    echo "Lin 20000"
    ./bin/client client_config5000.json >> client-omega-20k-64-new.txt
done

# for _ in {1..3}
# do
#     echo "Log 20000"
#     ./bin/client client_config5000.json >> client-omega-20k-famhe-64-new.txt
# done

# for _ in {1..3}
# do
#     echo "Log 20000"
#     ./bin/client client_config5000.json >> client-omega-20k-oblivious-64-new.txt
# done

# for _ in {1..3}
# do
#     echo "Lin 20000"
#     ./bin/client client_config5000.json >> client-omega-20k-oblivious-64-new-2.txt
# done

# for _ in {1..3}
# do
#     echo "Lin 20000"
#     ./bin/client client_config5000.json >> client-omega-20k-famhe-64-new.txt
# done