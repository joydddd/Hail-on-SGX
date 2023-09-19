#!/bin/bash
for _ in {1..7}
do
    echo "Log 5000"
    ./bin/client client_config1250-2.json >> testing-5k-2.txt
done

for _ in {1..7}
do
    echo "Lin 5000"
    ./bin/client client_config1250-2.json >> testing-5k-2.txt
done

# for _ in {1..7}
# do
#     echo "Log 10000"
#     ./bin/client client_config2500.json >> scaling-20k-2024-2.txt
# done

# for _ in {1..7}
# do
#     echo "Lin 10000"
#     ./bin/client client_config2500.json >> scaling-20k-2024-2.txt
# done

# for _ in {1..7}
# do
#     echo "Log 20000"
#     ./bin/client client_config5000.json >> scaling-20k-2024-2.txt
# done

# for _ in {1..7}
# do
#     echo "Lin 20000"
#     ./bin/client client_config5000.json>> scaling-20k-2024-2.txt
# done

# for _ in {1..7}
# do
#     echo "Log 20000"
#     ./bin/client client_config5000.json >> scaling-20k-famhe-2024-2.txt
# done

# for _ in {1..7}
# do
#     echo "Lin 20000"
#     ./bin/client client_config5000.json>> scaling-20k-famhe-2024-2.txt
# done

# for _ in {1..7}
# do
#     echo "Log 20000"
#     ./bin/client client_config5000.json >> scaling-20k-oblivious-2024-2.txt
# done

# for _ in {1..7}
# do
#     echo "Lin 20000"
#     ./bin/client client_config5000.json>> scaling-20k-oblivious-2024-2.txt
# done

