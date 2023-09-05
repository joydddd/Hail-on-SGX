#!/bin/bash

sudo apt update -y
sudo apt install python3-pip -y
sudo apt install python3.8-venv -y
sudo apt install default-jre -y
python3 -m venv env
source env/bin/activate
pip3 install -r requirements.txt
curl https://gist.githubusercontent.com/danking/f8387f5681b03edc5babdf36e14140bc/raw/23d43a2cc673d80adcc8f2a1daee6ab252d6f667/install-s3-connector.sh | bash