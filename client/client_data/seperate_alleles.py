FILE = "generated_alleles_2626-10000.tsv"

na_count = 0

def split_list(a_list):
    half = len(a_list)//2
    return a_list[:half], a_list[half:]

with open(FILE, 'r') as r:
    with open(FILE + "1", 'w') as f1:
        with open(FILE + "2", 'w') as f2:
            f1.write('bleh\n')
            f2.write('bleh\n')
            r.readline()
            for line in r:
                sl = line.split('\t')
                sl[-1] = sl[-1][:-1]

                locus = sl[0]
                allele = sl[1]
                
                A, B = split_list(sl[2:])
                s = '\t'
                f1.write(f'{locus}\t{allele}\t{s.join(A)}\n')
                f2.write(f'{locus}\t{allele}\t{s.join(B)}\n')
    
