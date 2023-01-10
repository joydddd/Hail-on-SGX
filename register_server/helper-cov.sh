# read n
# read x

x=$(($2 * 100000))

for cov in $1
do 
    for count in $(eval echo {$x..2000000..100000})
    do
        ./bin/register_server register_config.json >> output-$cov.txt
    done
done