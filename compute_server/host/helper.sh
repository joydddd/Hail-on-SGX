for VAR in {1..3..1}
do
    echo "Baseline"
    ./gwashost compute_server_config10000log.json >> sgx-c.txt
    ./gwashost compute_server_config10000lin.json >> sgx-c.txt
    echo "62.5k"
    ./gwashost compute_server_config10000log.json >> sgx-c-62.5.txt
    ./gwashost compute_server_config10000lin.json >> sgx-c-62.5.txt
    echo "250k"
    ./gwashost compute_server_config10000log.json >> sgx-c-250.txt
    ./gwashost compute_server_config10000lin.json >> sgx-c-250.txt
    echo "3 cov"
    ./gwashost compute_server_config10000log-3cov.json >> sgx-c-3cov.txt
    ./gwashost compute_server_config10000lin-3cov.json >> sgx-c-3cov.txt
    echo "12 cov"
    ./gwashost compute_server_config10000log-12cov.json >> sgx-c-12cov.txt
    ./gwashost compute_server_config10000lin-12cov.json >> sgx-c-12cov.txt
    echo "5k"
    ./gwashost compute_server_config5000log.json >> sgx-c5k.txt
    ./gwashost compute_server_config5000lin.json >> sgx-c5k.txt
    echo "20k"
    ./gwashost compute_server_config20000log.json >> sgx-c20k.txt
    ./gwashost compute_server_config20000lin.json >> sgx-c20k.txt
done

# for VAR in {1..3..1}
# do
#     echo "Baseline"
#     ./gwas_nonoe compute_server_config10000log.json >> spec-mit-bmc.txt
#     ./gwas_nonoe compute_server_config10000lin.json >> spec-mit-bmc.txt
#     echo "62.5k"
#     ./gwas_nonoe compute_server_config10000log.json >> spec-mit-bmc-62.5.txt
#     ./gwas_nonoe compute_server_config10000lin.json >> spec-mit-bmc-62.5.txt
#     echo "250k"
#     ./gwas_nonoe compute_server_config10000log.json >> spec-mit-bmc-250.txt
#     ./gwas_nonoe compute_server_config10000lin.json >> spec-mit-bmc-250.txt
#     echo "3 cov"
#     ./gwas_nonoe compute_server_config10000log-3cov.json >> spec-mit-bmc-3cov.txt
#     ./gwas_nonoe compute_server_config10000lin-3cov.json >> spec-mit-bmc-3cov.txt
#     echo "12 cov"
#     ./gwas_nonoe compute_server_config10000log-12cov.json >> spec-mit-bmc-12cov.txt
#     ./gwas_nonoe compute_server_config10000lin-12cov.json >> spec-mit-bmc-12cov.txt
#     echo "5k"
#     ./gwas_nonoe compute_server_config5000log.json >> spec-mit-bmc5k.txt
#     ./gwas_nonoe compute_server_config5000lin.json >> spec-mit-bmc5k.txt
#     echo "20k"
#     ./gwas_nonoe compute_server_config20000log.json >> spec-mit-bmc20k.txt
#     ./gwas_nonoe compute_server_config20000lin.json >> spec-mit-bmc20k.txt
# done