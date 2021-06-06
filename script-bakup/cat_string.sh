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

echo $CBB_LEVEL
echo $CBB_RELEASE

if [ "$CBB_RELEASE" == "JANE" ]
then
  echo CBB_RELEASE is $CBB_RELEASE
fi

if [ "$CBB_RELEASE" == "JUDY" ]
then
  echo CBB_RELEASE is $CBB_RELEASE
fi

char='abcde'
my_char=${char:5:2}
echo $my_char

