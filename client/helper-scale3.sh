#!/bin/bash
for _ in {1..7}
do
    echo "Log 5000"
    ./bin/client client_config1250-3.json >> testing-5k-3.txt
done

for _ in {1..7}
do
    echo "Lin 5000"
    ./bin/client client_config1250-3.json >> testing-5k-3.txt
done

# for _ in {1..7}
# do
#     echo "Log 10000"
#     ./bin/client client_config2500.json >> scaling-30k-3024-3.txt
# done

# for _ in {1..7}
# do
#     echo "Lin 10000"
#     ./bin/client client_config2500.json >> scaling-30k-3024-3.txt
# done

# for _ in {1..7}
# do
#     echo "Log 20000"
#     ./bin/client client_config5000.json >> scaling-30k-3024-3.txt
# done

# for _ in {1..7}
# do
#     echo "Lin 20000"
#     ./bin/client client_config5000.json>> scaling-30k-3024-3.txt
# done

# for _ in {1..7}
# do
#     echo "Log 20000"
#     ./bin/client client_config5000.json >> scaling-30k-famhe-3024-3.txt
# done

# for _ in {1..7}
# do
#     echo "Lin 20000"
#     ./bin/client client_config5000.json>> scaling-30k-famhe-3024-3.txt
# done

# for _ in {1..7}
# do
#     echo "Log 20000"
#     ./bin/client client_config5000.json >> scaling-30k-oblivious-3024-3.txt
# done

# for _ in {1..7}
# do
#     echo "Lin 20000"
#     ./bin/client client_config5000.json>> scaling-30k-oblivious-3024-3.txt
# done

