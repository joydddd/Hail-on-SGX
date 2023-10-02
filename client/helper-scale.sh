#!/bin/bash

for _ in {1..1}
do
    echo "Log 100000"
    ./bin/client client_config100000.json >> client-big.txt
done

for _ in {1..1}
do
    echo "Lin 100000"
    ./bin/client client_config100000.json >> client-big.txt
done

# for _ in {1..5}
# do
#     echo "Log 5000"
#     ./bin/client client_config1250.json >> client-omega-5k-512-LAN.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 5000"
#     ./bin/client client_config1250.json >> client-omega-5k-512-LAN.txt
# done

# for _ in {1..5}
# do
#     echo "Log 10000"
#     ./bin/client client_config2500.json >> client-omega-10k-512-LAN.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 10000"
#     ./bin/client client_config2500.json >> client-omega-10k-512-LAN.txt
# done

# for _ in {1..5}
# do
#     echo "Log 20000"
#     ./bin/client client_config5000.json >> client-omega-20k-512-LAN.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 20000"
#     ./bin/client client_config5000.json >> client-omega-20k-512-LAN.txt
# done

# for _ in {1..5}
# do
#     echo "Log 20000"
#     ./bin/client client_config5000.json >> client-omega-20k-famhe-512-LAN.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 20000"
#     ./bin/client client_config5000.json >> client-omega-20k-famhe-512-LAN.txt
# done

# for _ in {1..5}
# do
#     echo "Log 20000"
#     ./bin/client client_config5000.json >> client-omega-20k-oblivious-512-LAN.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 20000"
#     ./bin/client client_config5000.json >> client-omega-20k-oblivious-512-LAN-2.txt
# done
