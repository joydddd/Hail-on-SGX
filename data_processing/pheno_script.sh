for count in {100000..2000000..100000}
do 
    for cov in {0..16}
    do
        time python3 generate_phenotypes.py $count $cov &
    done
done
    
    