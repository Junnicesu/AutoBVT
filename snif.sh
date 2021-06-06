#!/bin/bash

cd /cygdrive/d/AutoBVT/remoteuxso/

i_var=90
while [[ $i_var -lt 254 ]];
do
  echo $i_var
  hostIP=$1.$i_var
  /cygdrive/c/Windows/system32/ping ${hostIP} -n 2
  if [ $? == 0 ] 
  then
    /cygdrive/d/AutoBVT/remoteuxso/rX.bin ${hostIP} root SYS2009health /home/
	if [ $? == 0 ]
	then
    /cygdrive/d/AutoBVT/remoteux_win/rX.exe ${hostIP} administrator SYS2009health C: "dir c: "
    fi
	
  fi

  
  ((i_var++))  
  
done