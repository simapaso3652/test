#!/bin/sh

echo "initAll.sh -------------"

koFile=addTag_INPacket.c
setting=setting.txt
ips=()
users=()
paths=()
serverNum=0
cnt=0

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
done


for i in `seq 0 $serverNum `
do
  #transport files
  tar czf - -C ./ kernelModuleTest.c | ssh root@192.168.11.4 'tar zxvf - -C /home/'
  tar czf - -C ./ test.c | ssh root@192.168.11.4 'tar zxvf - -C /home/'
done

echo "init.sh end-----------------------------"

echo "if ssh error occur: host key verification failed.
type ssh-keygen -R ${ips[$1]}"

