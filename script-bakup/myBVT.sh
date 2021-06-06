TARGETS="rhel5 sles11"
CBB_LEVEL=JANE08AUS-1348
LOCAL_TAR_PATH=/cygdrive/d/AutoBVT/BVT_BIN/${CBB_LEVEL}


for testTarget in $TARGETS
do
# Download 
#  echo Testing $testTarget  ...
  mkdir /cygdrive/d/AutoBVT/BVT_BIN/JANE08AUS-1348
  zipTar=$testTarget*.zip
  sepTar=ibm_utl_cbbsep_*$testTarget*i386.zip
  /cygdrive/d/AutoBVT/cpSrc.bat toolscbb_jane-pcclab-d32f JANE08AUS-1348/US /cygdrive/d/AutoBVT/BVT_BIN/JANE08AUS-1348 $zipTar
  /cygdrive/d/AutoBVT/cpSrc.bat JANE08AUS-1348/US/sep /cygdrive/d/BOSS/BVT_BIN/JANE08AUS-1348 $sepTar
# Download

# deploy
#  cd ${LOCAL_TAR_PATH}
#  sepTar=ibm_utl_cbbsep_1.00_$testTarget_i386.zip
#  unzip -n ${testTarget}.zip -d tmp
#  unzip -n ibm_utl_cbbsep_1.00_${testTarget}_i386.zip -d tmp
#  mv -f -u tmp/cbb/* tmp/${testTarget}/image/
#  mv -f -u tmp/${testTarget}/lib/* tmp/${testTarget}/image/
#  scp -rpC tmp/${testTarget}/image/ root@9.125.90.117:/home/${CBB_LEVEL}
#  ssh -t root@9.125.90.117 "/home/${CBB_LEVEL}/cbbcli "
# deploy

#remote execute
#  hostIP=9.125.90.188
#  ls -l ${LOCAL_TAR_PATH}/tmp/${testTarget}/image/
  /cygdrive/d/AutoBVT/remoteuxso/rX.bin ${hostIP} root SYS2009health /home/winson/tools/all/rhel5_64/instrumentation/ "cd /home/winson/tools/all/rhel5_64/instrumentation/; /home/winson/tools/all/rhel5_64/instrumentation/cbbcli -test output=/home/winson/output.html deltaspec=/home/winson/deltaspec.xml compare=/home/winson/compare.xml.gz"  
#remote execute


done
