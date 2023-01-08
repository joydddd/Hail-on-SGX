read n
read x

x=$((x * 100000))

for cov in $n
do 
    for count in $(eval echo {$x..2000000..100000})
    do
        ./bin/register_server register_config.json >> output-$cov.txt
    done
done