pkill gwas
pkill client
sleep 2
cd server/host
./gwashost 6502 ../enclave/gwasenc.signed &
cd ../../client
sleep 1.75
./bin/client client1 localhost 8089 localhost 6502