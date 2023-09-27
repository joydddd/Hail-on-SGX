for VAR in {1..5..1}
do
    echo "1"
    ./bin/client client_config10000.json
    ./bin/client client_config10000.json

    echo "2"
    ./bin/client client_config10000.json
    ./bin/client client_config10000.json

    echo "4"
    ./bin/client client_config10000.json
    ./bin/client client_config10000.json

    echo "8"
    ./bin/client client_config10000.json
    ./bin/client client_config10000.json

    echo "16"
    ./bin/client client_config10000.json
    ./bin/client client_config10000.json

    echo "32"
    ./bin/client client_config10000.json
    ./bin/client client_config10000.json

    echo "64"
    ./bin/client client_config10000.json
    ./bin/client client_config10000.json

    echo "128"
    ./bin/client client_config10000.json
    ./bin/client client_config10000.json

    echo "256"
    ./bin/client client_config10000.json
    ./bin/client client_config10000.json

    echo "512"
    ./bin/client client_config10000.json
    ./bin/client client_config10000.json

    echo "1024"
    ./bin/client client_config10000.json
    ./bin/client client_config10000.json
    
done
