count = 0
with open('alleles.tsv', 'r') as fread, open('alleles-multiplied.tsv', 'w') as fwrite:
    # Copy header line once!
    fwrite.write(fread.readline())
    for line in fread:
        for _ in range(10):
            count += 1
            fwrite.write(line)

print('Total alleles in new tsv: ' + str(count))
