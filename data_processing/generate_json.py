for count in range(100000, 1600000, 100000):
    for cov in range(1, 17, 1):
        with open(f'../compute_server/host/compute_server_config-{cov}-{count}.json', 'w') as f:
            f.write('{\n')
            f.write('\t"compute_server_bind_port": 6708,\n')
            cov_str = ""
            for i in range(cov):
                cov_str += f'"{i+1}-{count}", '
            f.write(f'\t"covariants": [{cov_str[:-2]}],\n')
            f.write('\t"institutions": ["client1"],\n')
            f.write(f'\t"y_val_name": "disease-{count}",\n')
            f.write('\t"analysis_type": "linear",\n')
            f.write('\t"enclave_path": "../enclave/gwasenc.signed",\n')
            f.write('\t"register_server_info": { "hostname": "20.237.195.214", "port": 6405 }\n')
            f.write('}')
