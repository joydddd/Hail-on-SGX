for cov in {1..16}
do 
    echo Covariants: $cov
    for count in {100000..2000000..100000}
    do
        ./bin/client client_config-$count.json
    done
done