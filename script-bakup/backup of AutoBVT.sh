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

#Begin the BVT TESTing LOOP
IPArray=(IP1 IP2 IP3 IP4 IP5 IP6 IP7 IP8 IP9)
IPArray[0]=9.125.90.117 #RHEL5 64
IPArray[1]=9.125.90.126 #RHEL3  32
IPArray[2]=9.125.90.129 #Rhel5 32
IPArray[3]=9.125.90.178 #SLES10  32
IPArray[4]=9.125.90.188 #RHEL5 64
IPArray[5]=9.125.90.35 #Rhel4 32
IPArray[6]=9.125.90.5 #SLES11 32
IPArray[7]=9.125.90.62 #RHEL4 64
IPArray[8]=9.125.90.86 #SLSE10 64
i_var=0
 
#TARGETS="rhel3_64 rhel3 rhel4_64 rhel4 rhel5_64 rhel5 rhel6_64 rhel6 sles9_64 sles9 sles10_64 sles10 sles11_64 sles11 win64 win"
#TARGETS="sles11_64 rhel3 rhel5 sles10 rhel5_64 rhel4 sles11 rhel4_64 sles10_64"
TARGETS="rhel5_64 "
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
    
#  /cygdrive/d/AutoBVT/cpSrc.bat ${SRCBASE_PATH} ${CBB_LEVEL}/US ${WIN_LOCAL_TAR_PATH} $zipTar
#  /cygdrive/d/AutoBVT/cpSrc.bat ${SRCBASE_PATH} ${CBB_LEVEL}/US/sep ${WIN_LOCAL_TAR_PATH} $sepTar
# Download

# deploy
  cd ${LOCAL_TAR_PATH}
  unzip -n ${zipTar} -d tmp
  unzip -n ${sepTar} -d tmp
  mv -f -u tmp/cbb/* tmp/${testTarget}/image/
  mv -f -u tmp/${testTarget}/lib/* tmp/${testTarget}/image/
# deploy

#remote execute
  hostIP=${IPArray[$i_var]}
#  ls -l ${LOCAL_TAR_PATH}/tmp/${testTarget}/image/
#  /cygdrive/d/AutoBVT/remoteuxso/rX.bin ${hostIP} root SYS2009health /home/winson/tools/all/rhel5_64/instrumentation/ "cd /home/winson/tools/all/rhel5_64/instrumentation/; /home/winson/tools/all/rhel5_64/instrumentation/cbbcli -test output=/home/winson/output.html deltaspec=/home/winson/deltaspec.xml compare=/home/winson/compare.xml.gz"  
  /cygdrive/d/AutoBVT/remoteuxso/rX.bin ${hostIP} root SYS2009health /home/ "cd /home/${testTarget}; ./cbbcli -test output=/home/${testTarget}/output.html deltaspec=/home/deltaspec.xml compare=/home/compare.xml.gz" -t ${LOCAL_TAR_PATH}/tmp/${testTarget}/image/ -b /home/${testTarget}/output.html  
  ((i_var++))  
#remote execute


done
