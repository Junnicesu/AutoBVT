#!/bin/bash
if [ $# -lt 1 ]
then 
  echo "Usage:"
  echo "mkLnk.sh <folder>"
  exit 1
else
  echo "Create Hard Links under: $1"
  THE_BLD_FDR=$1
fi

cd $THE_BLD_FDR
mkdir Lnk
cd Lnk
find ../ -name *.zip -exec ln -f -t . {} \;
find ../ -name *.iso -exec ln -f -t . {} \;




