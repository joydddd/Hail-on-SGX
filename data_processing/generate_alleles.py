import random
import numpy as np
import sys
import multiprocessing
import subprocess
import glob
import os
import math

random.seed('0x8BADF00D')

rand_size = 100003

rands = np.random.random(rand_size)

ALLELE_COUNT =  125000 // 8
CLIENT_COUNT = 10000 if len(sys.argv) != 2 else int(sys.argv[1])
print(CLIENT_COUNT)
NUM_PROCS = multiprocessing.cpu_count()
OUTPUT_FILE = f"../client/client_data/generated_alleles_{CLIENT_COUNT}-{ALLELE_COUNT}.tsv"

def run_bash_cmd(cmd):
    process = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE)
    process.communicate()

def weighted_random_by_dct(dct):
    rand_val = random.random()
    total = 0
    for k, v in dct.items():
        total += v
        if rand_val <= total:
            return k
    assert False, 'unreachable'

def get_random_allele(rand_val):
    if rand_val <= 0.4743076415612974:
        return '0'
    elif rand_val <= 0.7303159978009895:
        return '1'
    elif rand_val <= 0.9497530511269928:
        return '2'
    else:
        return 'NA'

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

proc_div = num_alleles / NUM_PROCS

# with open(f'../client/client_data/generated_alleles_{CLIENT_COUNT}.tsv', 'w') as f:

def helper(pid, locuses):
    random.seed('0x8BADF00D')
    index = pid * int(proc_div) * scale_up_factor * CLIENT_COUNT
    with open(f'tmp-{CLIENT_COUNT}-{str(pid).zfill(7)}.txt', 'w') as f:
        top_line = ""
        for i in range(CLIENT_COUNT):
            top_line += f'HG{i} '
        if pid == 0:
            f.write(f'locus\talleles {top_line[:-1]}\n')

        for loc_idx, locus in enumerate((locuses[:ALLELE_COUNT] if len(sys.argv) != 2 else locuses[:1])):
            if int(loc_idx / proc_div) != pid:
                continue 
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
                for i in range(CLIENT_COUNT):
                    genotype_data += get_random_allele(rands[index % rand_size]) + '\t'
                    index += 1
                f.write(f'{locus_split[0]}:{str(s_num)}\t{random_allele}\t{genotype_data[:-1]}\n')

ps = []

for pid in range(NUM_PROCS):
    p = multiprocessing.Process(target=helper, args=(pid, locuses,))
    p.start()
    ps.append(p)

for p in ps:
    p.join()

read_files = sorted(glob.glob(f'tmp-{CLIENT_COUNT}-*.txt'))

with open(OUTPUT_FILE, 'wb') as outfile:
    for f in read_files:
        with open(f, 'rb') as infile:
            outfile.write(infile.read())
        os.remove(f)

# for path in glob.glob(f'tmp-{CLIENT_COUNT}-*.txt'):
#     os.remove(path)