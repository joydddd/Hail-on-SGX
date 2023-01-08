read n
read x

x=$((x * 100000))

for cov in $n
do 
    echo Covariants: $cov
    for count in $(eval echo {$x..2000000..100000})
    do
        ./bin/client client_config-$count.json >> output-$cov.txt
    done
done