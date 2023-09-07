#!/bin/bash
for _ in {1..5}
do
    echo "Log 5000"
    ./bin/client client_config1250-4.json >> testing-5k-4.txt
done

for _ in {1..5}
do
    echo "Lin 5000"
    ./bin/client client_config1250-4.json >> testing-5k-4.txt
done

# for _ in {1..5}
# do
#     echo "Log 10000"
#     ./bin/client client_config2500.json >> scaling-10k-1024-4.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 10000"
#     ./bin/client client_config2500.json >> scaling-10k-1024-4.txt
# done

# for _ in {1..5}
# do
#     echo "Log 20000"
#     ./bin/client client_config5000.json >> scaling-40k-1024-4.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 20000"
#     ./bin/client client_config5000.json>> scaling-40k-1024-4.txt
# done

# for _ in {1..5}
# do
#     echo "Log 20000"
#     ./bin/client client_config5000.json >> scaling-40k-famhe-1024-4.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 20000"
#     ./bin/client client_config5000.json>> scaling-40k-famhe-1024-4.txt
# done

# for _ in {1..5}
# do
#     echo "Log 20000"
#     ./bin/client client_config5000.json >> scaling-40k-oblivious-1024-4.txt
# done

# for _ in {1..5}
# do
#     echo "Lin 20000"
#     ./bin/client client_config5000.json>> scaling-40k-oblivious-1024-4.txt
# done

