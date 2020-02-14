. /etc/os-release

if [ "$NAME" = "ubuntu" ]; then
    sudo apt-get update
    sudo apt-get -y install build-essential cmake libgflags2.2 libgflags-dev
    ./scripts/install-gtest.sh
    sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)" # install llvm, https://apt.llvm.org/
fi
