#!/bin/sh

# 必要なライブラリ check
echo "check requested library"
# mpi
mpirun > /dev/null 2>&1
#if [ $? -eq 127 ]; then
  # コマンドをインストールする処理とか
  #echo "you need to install mpi before init.sh"
  #return 1
  echo 
#fi
# kernel-devel
#if [ $? -eq 127 ]; then
  # コマンドをインストールする処理とか
 # echo "you need to install kernel-devel before init.sh"
 # return 1
#fi

echo "checkok!!"
echo ""
