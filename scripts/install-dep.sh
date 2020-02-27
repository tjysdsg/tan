. /etc/os-release

if [ "$NAME" = "ubuntu" ]; then
    sudo apt-get update
    sudo apt-get -y install build-essential cmake
    sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)" # install llvm, https://apt.llvm.org/
fi

git submodule init
git submodule update --recursive
