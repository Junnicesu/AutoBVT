#!/bin/bash

cd /cygdrive/d/AutoBVT/

i_var=2
i_ret=0

while [[ $i_var -lt 254 ]];
do
  echo $i_var
  hostIP=$1.$i_var
  /cygdrive/c/Windows/system32/ping ${hostIP} -n 2 > NULL 2>&1
  if [ $? == 0 ] 
  then
    echo "Scan linux ..."
    ./remoteuxso/IPscan.bin ${hostIP} 
    i_ret=$?
#	cd ..
	
#	if [ $i_ret != 0 ]
#	then
#	echo "Scan Win  ..."
#	cd ./remoteux_win/
#	./IPscan.exe ${hostIP} 
#    i_ret=$?
#	cd ..
#	fi
	
#	if [ $i_ret != 0 ]
#	then
#	echo "Scan IMM ..."
#    ./immscan.exe ${hostIP} 
#	pwd
#  fi
	
fi

  ((i_var++))  
  
done