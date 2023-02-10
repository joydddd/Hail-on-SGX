#!/bin/bash
echo 'deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu bionic main' | sudo tee /etc/apt/sources.list.d/intel-sgx.list
wget -qO - https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key | sudo apt-key add -

echo "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-7 main" | sudo tee /etc/apt/sources.list.d/llvm-toolchain-bionic-7.list
wget -qO - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -

echo "deb [arch=amd64] https://packages.microsoft.com/ubuntu/18.04/prod bionic main" | sudo tee /etc/apt/sources.list.d/msprod.list
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
sudo apt install gdb -y


sudo apt -y install dkms
sudo wget https://download.01.org/intel-sgx/sgx-dcap/1.7/linux/distro/ubuntu18.04-server/sgx_linux_x64_driver_1.35.bin -O sgx_linux_x64_driver.bin
sudo chmod +x sgx_linux_x64_driver.bin
sudo ./sgx_linux_x64_driver.bin

sudo apt -y install clang-10 libssl-dev gdb libsgx-enclave-common libsgx-quote-ex libprotobuf10 libsgx-dcap-ql libsgx-dcap-ql-dev az-dcap-client open-enclave

source /opt/openenclave/share/openenclave/openenclaverc

cd compute_server

make clean; make
make clean; make
# make clean; make nonoe

sudo apt install python3-pip -y
pip3 install numpy
pip3 install scipy