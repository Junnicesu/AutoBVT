#!/bin/bash
set -x 
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

if [ "$CBB_RELEASE" == "SKYW" ]
then
  SRCBASE_PATH=cbb_skywalker-pcclab-6785
fi

LOCAL_TAR_PATH=/cygdrive/d/BUILD/project/${CBB_RELEASE}/${CBB_LEVEL}
WIN_LOCAL_TAR_PATH=D:/BUILD/project/${CBB_RELEASE}/${CBB_LEVEL}
mkdir -p ${WIN_LOCAL_TAR_PATH}

#Begin the BVT TESTing LOOP
IPArray=(IP1 IP2 IP3 IP4 IP5 IP6 IP7 IP8 IP9 )
IPArray[0]=9.125.90.76 #Win64
IPArray[1]=9.125.90.25 #win
IPArray[2]=9.125.90.133 #RHEL5 32
IPArray[3]=9.125.90.62 #RHEL4 64
IPArray[4]=9.125.90.188 #RHEL5 64
IPArray[5]=9.125.90.35 #Rhel4 32
IPArray[6]=9.125.90.5 #SLES11 32
IPArray[7]=9.125.90.62 #RHEL4 64
IPArray[8]=9.125.90.86 #SLSE10 64
IPArray[9]=9.125.90.117 # sles11 64
IPArray[10]=9.125.90.126 #RHEL3  32
IPArray[11]=9.125.90.129 #Rhel5 32
IPArray[12]=9.125.90.178 #SLES10  32
IPArray[13]=9.125.90.188 #RHEL5 64
IPArray[14]=9.125.90.35 #Rhel4 32
IPArray[15]=9.125.90.5 #SLES11 32
IPArray[16]=9.125.90.62 #RHEL4 64
i_var=0

#Write ${PathSendMail}/BVTReport.cfg  head
PathSendMail=/cygdrive/d/AutoBVT/SendMail
mkdir -p ${PathSendMail}
echo "<?xml version=\"1.0\" encoding=\"utf-8\" ?> " > ${PathSendMail}/BVTReport.cfg
echo "<BVTReport version=\"1.0\">" >> ${PathSendMail}/BVTReport.cfg
#Write ${PathSendMail}/BVTReport.cfg  head
 
#TARGETS="rhel6 rhel3_64 rhel3 rhel4_64 rhel4 rhel5_64 rhel5 rhel6_64 sles9_64 sles9 sles10_64 sles10 sles11_64 sles11 win64 win "
#TARGETS="rhel5 rhel4 sles11_64 rhel4_64 sles10_64 sles11_64 rhel6 sles10"
TARGETS="rhel6 rhel6_64 rhel5 rhel5_64 win win64 sles10 sles10_64 sles11 sles11_64  "
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
    
  #copy $zipTar from \\bcrfss03.raleigh.ibm.com\SRCBASE\${SRCBASE_PATH}\${CBB_LEVEL}\US to ${WIN_LOCAL_TAR_PATH}
  #copy $sepTar from \\bcrfss03.raleigh.ibm.com\SRCBASE\${SRCBASE_PATH}\${CBB_LEVEL}\US\sep to ${WIN_LOCAL_TAR_PATH}
  
  /cygdrive/d/AutoBVT/cpSrc.bat ${SRCBASE_PATH} ${CBB_LEVEL}/US ${WIN_LOCAL_TAR_PATH} $zipTar
  /cygdrive/d/AutoBVT/cpSrc.bat ${SRCBASE_PATH} ${CBB_LEVEL}/US/sep ${WIN_LOCAL_TAR_PATH} $sepTar
# Download

#Write ${PathSendMail}/BVTReport.cfg content
  echo "  <OS name=\""${testTarget}"\" "ip="\""${IPArray[$i_var]}"\""" output="\"\\\\9.123.251.157\\build\\bvt\\"${CBB_RELEASE}"\\"${CBB_LEVEL}"\\"${testTarget}"\\IBM_Support\\DiffSummary.html\"" />"  >> ${PathSendMail}/BVTReport.cfg
#Write ${PathSendMail}/BVTReport.cfg content

#deployment & remote execute
  hostIP=${IPArray[$i_var]}
  echo Testing Remote target ${hostIP} ...
  cd /cygdrive/d/AutoBVT/remoteuxso/
  mkdir -p /cygdrive/d/BUILD/bvt/${CBB_RELEASE}/${CBB_LEVEL}/${testTarget}    
  if [ "$testTar_first" == "win" ]
  then
    /cygdrive/d/AutoBVT/winRX.bat ${hostIP} ${WIN_LOCAL_TAR_PATH}\\${zipTar} ${WIN_LOCAL_TAR_PATH}\\${sepTar} ${zip_SUFFIX}
    cp -rvf /cygdrive/d/AutoBVT/remoteux_win/tmp/IBM_Support /cygdrive/d/BUILD/bvt/${CBB_RELEASE}/${CBB_LEVEL}/${testTarget}
    rm -rvf /cygdrive/d/AutoBVT/remoteux_win/tmp/IBM_Support/*
  else
    /cygdrive/d/AutoBVT/remoteuxso/rX.bin ${hostIP} root SYS2009health /home/ "cd /home; /home/BVTCasesOnRt.sh ${testTarget}"  -t /cygdrive/d/AutoBVT/BVTCasesOnRt.sh /cygdrive/d/AutoBVT/deltaspec.xml ${LOCAL_TAR_PATH}/${zipTar} ${LOCAL_TAR_PATH}/${sepTar} -b /var/log/IBM_Support/
    cp -rvf /cygdrive/d/AutoBVT/remoteuxso/tmp/IBM_Support /cygdrive/d/BUILD/bvt/${CBB_RELEASE}/${CBB_LEVEL}/${testTarget}  
    rm -rvf /cygdrive/d/AutoBVT/remoteuxso/tmp/IBM_Support/*
  fi
  
  ((i_var++))  
#remote execute


done

#Write ${PathSendMail}/BVTReport.cfg  end
echo "</BVTReport> " >> ${PathSendMail}/BVTReport.cfg
#Write ${PathSendMail}/BVTReport.cfg  end

#send email
cd /cygdrive/d/AutoBVT/SendMail/
python BVTSendMail.py ${CBB_LEVEL}
#send email


echo "###########################################"

## Cp all.zip & the rest image zip finially
/cygdrive/d/AutoBVT/cpSrc.bat ${SRCBASE_PATH} ${CBB_LEVEL}/US ${WIN_LOCAL_TAR_PATH} *.zip /XF header.zip
/cygdrive/d/AutoBVT/cpSrc.bat ${SRCBASE_PATH} ${CBB_LEVEL}/US/sep ${WIN_LOCAL_TAR_PATH} *.zip /XF *candidate*.zip
/cygdrive/d/AutoBVT/cpSrc.bat ${SRCBASE_PATH} ${CBB_LEVEL} ${WIN_LOCAL_TAR_PATH} all.zip
/cygdrive/d/AutoBVT/cpSrc.bat ${SRCBASE_PATH} ${CBB_LEVEL}/BUILD ${WIN_LOCAL_TAR_PATH} makefile.lvl
## Cp all.zip & the rest image zip finially
