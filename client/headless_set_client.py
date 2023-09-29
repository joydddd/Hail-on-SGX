#!/usr/bin/python

import shutil
import os

with open('client_config100000.json', 'r') as fread, open('client_config100000.json.temp', 'w') as fwrite:
    for line in fread:
        if '"client_name"' in line:
            client_num = str(input("Input client number: "))
            line = '\t"client_name": "client' + client_num + '",\n'
        fwrite.write(line)

shutil.copyfile('client_config100000.json.temp', 'client_config100000.json')
os.remove('client_config100000.json.temp')