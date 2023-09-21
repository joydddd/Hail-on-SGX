#!/bin/bash
# for _ in {1..7}
# do
#     echo "Log 5000"
#     ./bin/client client_config1250.json >> client-omega-1-5k-1024.txt
# done

# for _ in {1..7}
# do
#     echo "Lin 5000"
#     ./bin/client client_config1250.json >> client-omega-1-5k-1024.txt
# done

# for _ in {1..7}
# do
#     echo "Log 10000"
#     ./bin/client client_config2500.json >> client-omega-1-10k-1024.txt
# done

# for _ in {1..7}
# do
#     echo "Lin 10000"
#     ./bin/client client_config2500.json >> client-omega-1-10k-1024.txt
# done

# for _ in {1..7}
# do
#     echo "Log 20000"
#     ./bin/client client_config5000.json >> client-omega-1-20k-1024.txt
# done

for _ in {1..7}
do
    echo "Lin 20000"
    ./bin/client client_config5000.json>> client-omega-1-20k-1024.txt
done

for _ in {1..7}
do
    echo "Log 20000"
    ./bin/client client_config5000.json >> client-omega-1-20k-famhe-1024.txt
done

for _ in {1..7}
do
    echo "Lin 20000"
    ./bin/client client_config5000.json>> client-omega-1-20k-famhe-1024.txt
done

for _ in {1..7}
do
    echo "Log 20000"
    ./bin/client client_config5000.json >> client-omega-1-20k-oblivious-1024.txt
done

for _ in {1..7}
do
    echo "Lin 20000"
    ./bin/client client_config5000.json>> client-omega-1-20k-oblivious-1024.txt
done

