for VAR in {1..5..1}
do
    echo "Baseline"
    ./bin/client client_config10000.json
    ./bin/client client_config10000.json
    echo "62.5k"
    ./bin/client client_config10000-62500.json
    ./bin/client client_config10000-62500.json
    echo "250k"
    ./bin/client client_config10000-250000.json
    ./bin/client client_config10000-250000.json
    echo "3 cov"
    ./bin/client client_config10000.json
    ./bin/client client_config10000.json
    echo "12 cov"
    ./bin/client client_config10000.json
    ./bin/client client_config10000.json
    echo "5k"
    ./bin/client client_config5000.json
    ./bin/client client_config5000.json
    echo "20k"
    ./bin/client client_config20000.json
    ./bin/client client_config20000.json
done