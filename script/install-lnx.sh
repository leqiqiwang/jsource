#!/bin/sh
sudo dpkg --add-architecture i386
sudo apt-get install -y build-essential gcc-multilib 
sudo apt-get install -y libedit-dev libncursesw5-dev
sudo apt-get install -y nasm
echo "fr_FR.UTF-8 UTF-8" | sudo tee -a /etc/locale.gen
sudo locale-gen
