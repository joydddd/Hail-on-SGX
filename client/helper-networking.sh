for VAR in {1..10..1}
do
    echo "1954"
    ./bin/client client_config10000-1954.json >> client_local_1.txt
    ./bin/client client_config10000-1954.json >> client_local_1.txt
    echo "3907"
    ./bin/client client_config10000-3907.json >> client_local_2.txt
    ./bin/client client_config10000-3907.json >> client_local_2.txt
    echo "7813"
    ./bin/client client_config10000-7813.json >> client_local_3.txt
    ./bin/client client_config10000-7813.json >> client_local_3.txt
    echo "15625"
    ./bin/client client_config10000-15625.json >> client_local_4.txt
    ./bin/client client_config10000-15625.json >> client_local_4.txt
    echo "31250"
    ./bin/client client_config10000-31250.json >> client_local_5.txt
    ./bin/client client_config10000-31250.json >> client_local_5.txt
    echo "62500"
    ./bin/client client_config10000-62500.json >> client_local_6.txt
    ./bin/client client_config10000-62500.json >> client_local_6.txt
done

for VAR in {1..7..1}
do
    echo "125000"
    ./bin/client client_config10000.json >> client_local_7.txt
    ./bin/client client_config10000.json >> client_local_7.txt
done

for VAR in {1..3..1}
do
    echo "500000"
    ./bin/client client_config10000-500000.json >> client_local_9.txt
    ./bin/client client_config10000-500000.json >> client_local_9.txt

    echo "250000"
    ./bin/client client_config10000-250000.json >> client_local_8.txt
    ./bin/client client_config10000-250000.json >> client_local_8.txt
done