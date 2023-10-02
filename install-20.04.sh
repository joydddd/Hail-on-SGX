#!/bin/bash

curl https://ipv4.icanhazip.com > ip.txt
cp ip.txt compute_server/host/ip.txt
cp ip.txt client/ip.txt
rm ip.txt

echo 'deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu focal main' | sudo tee /etc/apt/sources.list.d/intel-sgx.list
wget -qO - https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key | sudo apt-key add -

echo "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-10 main" | sudo tee /etc/apt/sources.list.d/llvm-toolchain-focal-10.list
wget -qO - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -

echo "deb [arch=amd64] https://packages.microsoft.com/ubuntu/20.04/prod focal main" | sudo tee /etc/apt/sources.list.d/msprod.list
wget -qO - https://packages.microsoft.com/keys/microsoft.asc | sudo apt-key add -

sudo apt update
sudo apt-get update -y
sudo apt-get install make -y
sudo apt-get install g++ -y
sudo apt-get install libcrypto++-dev libcrypto++-doc libcrypto++-utils -y
sudo apt install libmbedtls-dev -y
sudo apt-get install python -y
sudo apt-get install libcurl4-openssl-dev -y
sudo apt-get install libboost-all-dev -y
sudo apt-get install clang-10 libssl-dev gdb libsgx-enclave-common libsgx-quote-ex libprotobuf17 libsgx-dcap-ql libsgx-dcap-ql-dev az-dcap-client open-enclave -y

source /opt/openenclave/share/openenclave/openenclaverc

cd compute_server

make clean; make
make clean; make

cd ../client

make clean; make

cd ../register_server

make clean; make

sudo apt install python3-pip -y
pip3 install numpy
pip3 install scipy

cd ../data_processing

# python3 generate_alleles.py 1250 &
# python3 generate_alleles.py 2500 &
# python3 generate_alleles.py 5000 &

# python3 generate_phenotypes.py 1250 12
# python3 generate_phenotypes.py 1250 0

# python3 generate_phenotypes.py 2500 12
# python3 generate_phenotypes.py 2500 0

# python3 generate_phenotypes.py 5000 12
# python3 generate_phenotypes.py 5000 0

python3 generate_phenotypes.py 100000 12
python3 generate_phenotypes.py 100000 0

python3 generate_alleles.py &
python3 generate_alleles2.py &