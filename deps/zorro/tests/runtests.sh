#!/bin/bash
function run()
{
  ../build/zorro $1.zs >last.txt
  diff -q $1.ok last.txt
  if [ $? != 0 ];then
    echo $1.zs fail
    exit
  else 
    echo $1.zs ok
  fi
}

for i in \
  `ls -1 test*.zs|sort`
do
  run `basename $i .zs`
done
rm -f zorro.log
rm -f last.txt
