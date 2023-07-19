# for VAR in {1..3..1}
# do
#     echo "Baseline"
#     ./gwashost compute_server_config10000log.json >> sgx-no-mit.txt
#     ./gwashost compute_server_config10000lin.json >> sgx-no-mit.txt
#     echo "62.5k"
#     ./gwashost compute_server_config10000log.json >> sgx-no-mit-62.5.txt
#     ./gwashost compute_server_config10000lin.json >> sgx-no-mit-62.5.txt
#     echo "250k"
#     ./gwashost compute_server_config10000log.json >> sgx-no-mit-250.txt
#     ./gwashost compute_server_config10000lin.json >> sgx-no-mit-250.txt
#     echo "3 cov"
#     ./gwashost compute_server_config10000log-3cov.json >> sgx-no-mit-3cov.txt
#     ./gwashost compute_server_config10000lin-3cov.json >> sgx-no-mit-3cov.txt
#     echo "12 cov"
#     ./gwashost compute_server_config10000log-12cov.json >> sgx-no-mit-12cov.txt
#     ./gwashost compute_server_config10000lin-12cov.json >> sgx-no-mit-12cov.txt
#     echo "5k"
#     ./gwashost compute_server_config5000log.json >> sgx-no-mit5k.txt
#     ./gwashost compute_server_config5000lin.json >> sgx-no-mit5k.txt
#     echo "20k"
#     ./gwashost compute_server_config20000log.json >> sgx-no-mit20k.txt
#     ./gwashost compute_server_config20000lin.json >> sgx-no-mit20k.txt
# done

for VAR in {1..3..1}
do
    echo "Baseline"
    ./gwas_nonoe compute_server_config10000log.json >> nonoe.txt
    ./gwas_nonoe compute_server_config10000lin.json >> nonoe.txt
    echo "62.5k"
    ./gwas_nonoe compute_server_config10000log.json >> nonoe-62.5.txt
    ./gwas_nonoe compute_server_config10000lin.json >> nonoe-62.5.txt
    echo "250k"
    ./gwas_nonoe compute_server_config10000log.json >> nonoe-250.txt
    ./gwas_nonoe compute_server_config10000lin.json >> nonoe-250.txt
    echo "3 cov"
    ./gwas_nonoe compute_server_config10000log-3cov.json >> nonoe-3cov.txt
    ./gwas_nonoe compute_server_config10000lin-3cov.json >> nonoe-3cov.txt
    echo "12 cov"
    ./gwas_nonoe compute_server_config10000log-12cov.json >> nonoe-12cov.txt
    ./gwas_nonoe compute_server_config10000lin-12cov.json >> nonoe-12cov.txt
    echo "5k"
    ./gwas_nonoe compute_server_config5000log.json >> nonoe5k.txt
    ./gwas_nonoe compute_server_config5000lin.json >> nonoe5k.txt
    echo "20k"
    ./gwas_nonoe compute_server_config20000log.json >> nonoe20k.txt
    ./gwas_nonoe compute_server_config20000lin.json >> nonoe20k.txt
done