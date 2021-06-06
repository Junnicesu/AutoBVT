#!/bin/bash
set -x 

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
testTar_prefix=${testTarget:0:3}

if [ "$testTar_prefix" == "mcp" ]
then 
  echo "MCP test"
  sep_SUFFIX=_x86_64
  if [ "$testTar_suffix" == "61"  ]
  then
    sepTar=ibm_utl_cbbsep_1.00_mcp61_x86_64.zip
    zipTar=mcp61.zip
  else
    sepTar=ibm_utl_cbbsep_1.00_mcp_x86_64.zip
    zipTar=mcp50.zip
    testTarget=mcp50
  fi
else
  echo "Not For MCP test"
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
fi 

  
unzip -n ${zipTar} -d tmp 
unzip -n ${sepTar} -d tmp
alias cp=cp
cp -rf -u tmp/cbb/* tmp/${testTarget}/image/
cp -rf -u tmp/${testTarget}/lib/* tmp/${testTarget}/image/
cd tmp/${testTarget}/image/
./cbbcli collect -l /tmp -U
export DSA_LOGLEVEL=4
export DSA_LOGFILE=/var/log/IBM_Support/dsa_logfile
rm -rf /home/BVT/
mkdir -p /home/BVT/
mv -f -u /var/log/IBM_Support/*.xml.gz /home/BVT/compare.xml.gz
./cbbcli -test output=/var/log/IBM_Support/DiffSummary.html deltaspec=/home/deltaspec.xml compare=/home/BVT/compare.xml.gz
#cd ../../..
#rm -rf tmp
