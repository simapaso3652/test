#!/bin/sh

echo "install.sh ----------"
./check.sh

askYesOrNo() {
    while true ; do
        read -p "$1 (y/n)?" answer
        case $answer in
            [yY] | [yY]es | YES )
                return 0;;
            [nN] | [nN]o | NO )
                return 1;;
            * ) echo "Please answer yes or no.";;
        esac
    done
}


# MPIライブラリの置換
# ./extendMPILibrary.sh

# LSM_MPIAddtag のインストール
echo "meke menuconfigを実行します。システムを起動させたい場合には、LSM MPI addtagを選択してください。"
sudo make menuconfig

# コンパイル
#make security/
#make modules_install
