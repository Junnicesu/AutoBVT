#!/bin/bash
if [ $# -lt 1 ]
then
  echo "Usage:"
  echo "./AutoBVT.sh <LevelName>"
  exit 1
else
  echo "arg1: $1"
  CBB_LEVEL=$1
  CBB_RELEASE=${CBB_LEVEL:0:4}
fi

echo CBB_LEVEL is $CBB_LEVEL
echo CBB_RELEASE is $CBB_RELEASE

if [ "$CBB_RELEASE" == "JANE" ]
then
  SRCBASE_PATH=toolscbb_jane-pcclab-d32f
fi

if [ "$CBB_RELEASE" == "JUDY" ]
then
  SRCBASE_PATH=toolscbb_judy-pcclab-6a10
fi

LOCAL_TAR_PATH=/cygdrive/d/BUILD/project/${CBB_RELEASE}/${CBB_LEVEL}
WIN_LOCAL_TAR_PATH=D://BUILD//project//${CBB_RELEASE}//${CBB_LEVEL}
mkdir -p ${WIN_LOCAL_TAR_PATH}


TARGETS="rhel3_64 rhel3 rhel4_64 rhel4 rhel5_64 rhel5 rhel6_64 rhel6 sles9_64 sles9 sles10_64 sles10 sles11_64 sles11 win64 win"
for testTarget in $TARGETS
do

# Download 
  echo Testing $testTarget  ...
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

  if [ "$testTarget" == "win64" ]
  then
    testTar_first="win"
    zip_SUFFIX="64"
  fi
  
  
  zipTar=${testTar_first}${zip_SUFFIX}.zip
  sepTar=ibm_utl_cbbsep_1.00_${testTar_first}${sep_SUFFIX}.zip
    
  /cygdrive/d/AutoBVT/cpSrc.bat ${SRCBASE_PATH} ${CBB_LEVEL}/US ${WIN_LOCAL_TAR_PATH} $zipTar
  /cygdrive/d/AutoBVT/cpSrc.bat ${SRCBASE_PATH} ${CBB_LEVEL}/US/sep ${WIN_LOCAL_TAR_PATH} $sepTar
# Download

done
