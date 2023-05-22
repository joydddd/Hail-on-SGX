# read n
# read x

# x=$(($2 * 100000))

for i in {1..3..1}
do
    for cov in {1..16..3}
    do 
        for count in $(eval echo {100000..2000000..100000})
        do
            echo $cov $count >> output-$cov.txt
            ./bin/register_server register_config.json >> output-$cov.txt
        done
    done
done