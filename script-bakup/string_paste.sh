TARGETS="rhel5_64 rhel5 sles11_64 win64"
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
  
  zipTar=${testTar_first}${zip_SUFFIX}.zip
  sepTar=ibm_utl_cbbsep_*${testTar_first}${sep_SUFFIX}.zip
  
  echo zipTar is $zipTar
  echo sepTar is $sepTar
  
done
