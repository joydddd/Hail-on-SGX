#!/usr/bin/python
import multiprocessing
import shutil
import os

with open('gwas_enc.conf', 'r') as fread, open('gwas_enc.conf.temp', 'w') as fwrite:
    for line in fread:
        if 'NumTCS' in line:
            line = 'NumTCS=3\n'# + str(2 * multiprocessing.cpu_count()) + '\n'
        fwrite.write(line)

shutil.copyfile('gwas_enc.conf.temp', 'gwas_enc.conf')
os.remove('gwas_enc.conf.temp')