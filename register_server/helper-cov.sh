for cov in {1..16}
do 
    for count in {100000..2000000..100000}
    do
        ./bin/register_server register_config.json
    done
done