#!/bin/sh

koFile=addTag_INPacket.c
setting=setting.txt
ips=()
users=()
paths=()
serverNum=0
cnt=0
passwd=()

# 必要なライブラリ check
echo "check requested library"
# sshpass
# mpi 
mpirun -help > /dev/null 2>&1
if [ $? -eq 127 ]; then
  # コマンドをインストールする処理とか
  echo "you need to install mpi before init.sh"
  return 1
fi
# kernel-devel
if [ $? -eq 127 ]; then
  # コマンドをインストールする処理とか
  echo "you need to install kernel-devel before init.sh"
  return 1
fi

echo "check ok!!" 
echo "initAll.sh -------------------------------------------------------"

#ssh kakunin
for i in `cat $setting`
do
  serverNum=`expr $serverNum + 1`
  cnt=`expr $cnt + 1`
  if [ `expr $cnt % 2` = 0 ]; then
    users+=($i)
  elif [ `expr $cnt % 3` = 0 ]; then
    paths+=($i)
  else 
    ips+=($i)
  fi
  if [ $cnt = 3 ]; then
    cnt=0
  fi
done < $setting

serverNum=`expr $serverNum / 3 - 1 `

for i in `seq 0 $serverNum `
do
  echo "ip ${ips[$i]} "
  echo "user ${users[$i]} "
  echo "path ${paths[$i]} "
  echo "please type password for root login."
  read -sp "Password: " pass
  echo
  passwd+=($pass)
  #.check.sh
  echo "check yum library"
  sshpass -p ${passwd[$i]} ssh root@${ips[$i]} 'sh' < check.sh
done


for i in `seq 0 $serverNum `
do
  #transport files
  echo "transport libsource"
  tar czf - -C ./ libsource | sshpass -p ${passwd[$i]} ssh root@${ips[$i]} 'tar zxvf - -C /home'
  echo "touch result.txt" 
  sshpass -p ${passwd[$i]} ssh root@${ips[$i]} 'touch /home/libsource/result.txt'
done

echo "init.sh end-----------------------------"


