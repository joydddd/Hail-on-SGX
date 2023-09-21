for VAR in {1..10..1}
do
    echo "1954"
    ./bin/register_server register_config.json >> register_local_1.txt
    ./bin/register_server register_config.json >> register_local_1.txt
    echo "3907"
    ./bin/register_server register_config.json >> register_local_2.txt
    ./bin/register_server register_config.json >> register_local_2.txt
    echo "7813"
    ./bin/register_server register_config.json >> register_local_3.txt
    ./bin/register_server register_config.json >> register_local_3.txt
    echo "15625"
    ./bin/register_server register_config.json >> register_local_4.txt
    ./bin/register_server register_config.json >> register_local_4.txt
    echo "31250"
    ./bin/register_server register_config.json >> register_local_5.txt
    ./bin/register_server register_config.json >> register_local_5.txt
    echo "62500"
    ./bin/register_server register_config.json >> register_local_6.txt
    ./bin/register_server register_config.json >> register_local_6.txt
done

for VAR in {1..5..1}
do
    echo "125000"
    ./bin/register_server register_config.json >> register_local_7.txt
    ./bin/register_server register_config.json >> register_local_7.txt
done

for VAR in {1..3..1}
do
    echo "500000"
    ./bin/register_server register_config.json >> register_local_9.txt
    ./bin/register_server register_config.json >> register_local_9.txt

    echo "250000"
    ./bin/register_server register_config.json >> register_local_8.txt
    ./bin/register_server register_config.json >> register_local_8.txt
done