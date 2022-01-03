sudo apt update
sudo apt install python3-pip
sudo apt install python3.8-venv
sudo apt install default-jre
python3 -m venv env
source env/bin/activate
pip3 install -r requirements.txt
git config --global user.name "Jonah Rosenblum"
git config --global user.email jonaherosenblum@gmail.com