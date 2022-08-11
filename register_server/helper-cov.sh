for cov in {16..16}
do 
    for count in {100000..1500000..100000}
    do
        ./bin/register_server register_config.json
    done
done