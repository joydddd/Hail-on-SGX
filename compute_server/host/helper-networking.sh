for VAR in {1..10..1}
do
    echo "1954"
    ./gwashost compute_server_config10000log.json >> compute_local_1.txt
    ./gwashost compute_server_config10000lin.json >> compute_local_1.txt
    echo "3907"
    ./gwashost compute_server_config10000log.json >> compute_local_2.txt
    ./gwashost compute_server_config10000lin.json >> compute_local_2.txt
    echo "7813"
    ./gwashost compute_server_config10000log.json >> compute_local_3.txt
    ./gwashost compute_server_config10000lin.json >> compute_local_3.txt
    echo "15625"
    ./gwashost compute_server_config10000log.json >> compute_local_4.txt
    ./gwashost compute_server_config10000lin.json >> compute_local_4.txt
    echo "31250"
    ./gwashost compute_server_config10000log.json >> compute_local_5.txt
    ./gwashost compute_server_config10000lin.json >> compute_local_5.txt
    echo "62500"
    ./gwashost compute_server_config10000log.json >> compute_local_6.txt
    ./gwashost compute_server_config10000lin.json >> compute_local_6.txt
done

for VAR in {1..15..1}
do
    echo "125000"
    ./gwashost compute_server_config10000log.json >> compute_local_7.txt
    ./gwashost compute_server_config10000lin.json >> compute_local_7.txt
done

for VAR in {1..3..1}
do
    echo "500000"
    ./gwashost compute_server_config10000log.json >> compute_local_9.txt
    ./gwashost compute_server_config10000lin.json >> compute_local_9.txt

    echo "250000"
    ./gwashost compute_server_config10000log.json >> compute_local_8.txt
    ./gwashost compute_server_config10000lin.json >> compute_local_8.txt
done