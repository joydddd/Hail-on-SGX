import random

random.seed('0x8BADF00D')

ALLELE_COUNT = 10000
CLIENT_COUNT = 250

def weighted_random_by_dct(dct):
    rand_val = random.random()
    total = 0
    for k, v in dct.items():
        total += v
        if rand_val <= total:
            return k
    assert False, 'unreachable'

genotypes = {"0": 0, "1": 0, "2": 0, "NA": 0}
locuses = []
alleles = dict()

num_alleles = 0
with open('alleles.tsv', 'r') as f:
    f.readline()
    for line in f:
        num_alleles += 1
        line = line.replace('\n', '')
        split = line.split('\t')
        locuses.append(split[0])
        if split[1] not in alleles:
            alleles[split[1]] = 0
        alleles[split[1]] += 1
        for i in range(2,len(split)):
            genotypes[split[i]] += 1

geno_sum = 0
for key in genotypes:
    geno_sum += genotypes[key]

for key in genotypes:
   genotypes[key] = genotypes[key] / geno_sum

alleles_sum = 0
for key in alleles:
    alleles_sum += alleles[key]

for key in alleles:
    alleles[key] = alleles[key] / alleles_sum

scale_up_factor =  (ALLELE_COUNT // num_alleles) + 1

with open('generated_alleles.tsv', 'w') as f:
    top_line = ""
    for i in range(CLIENT_COUNT):
        top_line += f'HG{i} '
    f.write(f'locus\talleles {top_line[:-1]}')
    for locus in locuses:
        locus_split = locus.split(':')
        num = int(locus_split[1])
        nums = set()
        for i in range(scale_up_factor):
            while(True):
                random_num = random.randint(num - 7000, num + 7000)
                if random_num not in nums:
                    nums.add(random_num)
                    break
        sorted_nums = sorted(nums)
        for s_num in sorted_nums:
            random_allele = weighted_random_by_dct(alleles)
            genotype_data = ""
            for _ in range(CLIENT_COUNT):
                genotype_data += weighted_random_by_dct(genotypes) + '\t'
            f.write(f'{locus_split[0]}:{str(s_num)}\t{random_allele}\t{genotype_data[:-1]}\n')
            #locus_split[0] + ':' + str(s_num) + '\t' '\n')
    
    
    # diff = num - last_num
    # last_num = num
    # if first:
    # diff_sum += diff
