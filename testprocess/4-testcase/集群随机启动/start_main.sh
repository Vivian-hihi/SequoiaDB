#!/usr/bin/sh

#execute all testcase
sh start_test.sh > temp.txt

#calcute the fail test count
count=$(grep "execute fail" temp.txt | wc -l)


temp=$(grep "execute fail" temp.txt)

if [ $count -ne 0 ]; then
   echo "some testcase execute failed."
   echo "$temp"
else
   echo "all testcase execute success."
fi
