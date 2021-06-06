#!/bin/bash

cd /cygdrive/d/AutoBVT/remoteuxso/

i_var=2
while [[ $i_var -lt 254 ]];
do
  echo $i_var
  hostIP=192.168.0.$i_var
  ping ${hostIP} -n 2
  ((i_var++))  
  
done