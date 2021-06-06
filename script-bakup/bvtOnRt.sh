#!/bin/bash
echo The working dir is $PWD
if [ $# -lt 1 ]
then
  echo "Pls specfy your test target"
  exit
fi

testTarget=$1

intPos=${#testTarget}
(( intPos= (intPos - 2) ))
echo  $intPos
testTar_suffix=${testTarget:$intPos:2}

if [ "$testTar_suffix" == "64" ]
then
  ((intPos--))
  testTar_first=${testTarget:0:intPos}
  zip_SUFFIX=_64
  sep_SUFFIX=_x86_64
else
  zip_SUFFIX=""
  testTar_first=${testTarget}
  sep_SUFFIX=_i386
fi

zipTar=${testTar_first}${zip_SUFFIX}.zip
sepTar=ibm_utl_cbbsep_1.00_${testTar_first}${sep_SUFFIX}.zip
  
unzip -n ${zipTar} -d tmp 
unzip -n ${sepTar} -d tmp
alias cp=cp
cp -rf -u tmp/cbb/* tmp/${testTarget}/image/
cp -rf -u tmp/${testTarget}/lib/* tmp/${testTarget}/image/
cd tmp/${testTarget}/image/
./cbbcli collect -v -U
cd ../../..
rm -rf tmp
