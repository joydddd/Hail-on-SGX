for count in range(100000, 1600000, 100000):
    for cov in range(1, 17, 1):
        with open(f'../client/client_config-{count}.json', 'w') as f:
            f.write('{\n')
            f.write('\t"client_name": "client1",\n')
            f.write(f'\t"client_bind_port": 8601,\n')
            f.write(f'\t"allele_file": "client_data/generated_alleles_{count}.tsv",\n')
            f.write('\t"register_server_info": { "hostname": "20.42.86.216", "port": 6401 }\n')
            f.write('}')

# {
#     "client_name": "client1",
#     "client_bind_port": 8601,
#     "allele_file": "client_data/generated_alleles_2500.tsv",
#     "register_server_info": {
#         "hostname": "102.37.153.209",
#         "port": 6401
#     }
# }