for count in {100000..1500000..100000}
do
time python3 generate_alleles.py $count
done