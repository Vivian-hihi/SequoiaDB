#bin/bash

# define root path
testRoot="testcases/hlt/js_testcases/js"
sdbRoot="bin"
csprefix="local_test""$$"
coordsvcname="50000"

# define test parameter
testDir=$testRoot
testFile=""
stopWhenFailed=1

# define stat parameter
sucNum=0
failedNum=0
useTime=0
beginTime=0
endTime=0
beginTimeSec=0
endTimeSec=0

# define ignore path and file
pathArray=("vote")
fileArray=("libs.js")

# common function
function display()
{
   echo "run testcase 1.0.0 2014/2/25"
   echo "$0 --help"
   echo "$0 [-p path]|[-f file] [-s stopFlag] [-n svcname]"
   echo ""
   exit $1
}

function getMyPath()
{
   if [ ${1:0:1} = "/" ] ; then
      echo "$1"
   else
      echo "$testRoot/$1"
   fi
}

function showResult()
{
   echo "***********************************************************"
   echo "                    ***test result*** "
   echo " begin time: $beginTime"
   echo " end time  : $endTime"
   echo " use time  : `expr $endTimeSec - $beginTimeSec`(secs)"
   echo " total     : `expr $sucNum + $failedNum`"
   echo -n " succeed   :"
   echo -e "\033[32;49;1m $sucNum \033[39;49;0m"
   echo -n " failed    :"
   if [ $failedNum -ne 0 ] ; then
      echo -e "\033[31;49;1m $failedNum \033[39;49;0m"
   else
      echo " $failedNum"
   fi
   echo "***********************************************************"
}

# ***************************************************************
# run entry
# ***************************************************************
if [ $# -eq 1 -a "$1" = "--help" ] ; then
   display 0
fi

# loop all parameter
p=""
readType=0 # 1: path, 2: file, 3: stopWhenFailed, 4: svcname

for p in $@
do
   if [ $readType -eq 1 ] ; then
      testDir="$(getMyPath $p)"
      readType=0
   elif [ $readType -eq 2 ] ; then
      testFile="$(getMyPath $p)"
      readType=0
   elif [ $readType -eq 3 ] ; then
      stopWhenFailed=$(($p))
      readType=0
   elif [ $readType -eq 4 ] ; then
      coordsvcname="$p"
      readType=0
   elif [ "$p" = "-p" ] ; then
      readType=1 ;
   elif [ "$p" = "-f" ] ; then
      readType=2
   elif [ "$p" = "-s" ] ; then
      readType=3
   elif [ "$p" = "-n" ] ; then
      readType=4
   else
      echo "invalid arguments: $p"
      display 1
   fi
done

if [ $readType -ne 0 ] ; then
   echo "invalid arguments"
   display 1
fi

if [ "$testFile" != "" ] ; then
   testDir="$testFile"
fi

if [ "$testDir" != "$testRoot" ] ; then
   unset pathArray
   unset fileArray
fi

# construct exclude dirs and exclude files
pathString=""
fileString=""
findCmdStr="find $testDir "
beginPrefix=""
endPrefix=""

for data in ${pathArray[@]}
do
   if [ "$pathString" != "" ] ; then
      pathString=${pathString}" -o "
   fi
   pathString=${pathString}"-path ""\""${data}"\""
   beginPrefix=" "
   endPrefix=" -prune -o  "
done

for data in ${fileArray[@]}
do
   if [ "$fileString" != "" -o "$pathString" != "" ] ; then
      fileString=${fileString}" -o "
   fi
   fileString=${fileString}"-name ""\""${data}"\""
   beginPrefix=" "
   endPrefix=" -prune -o "
done

# construct find command
findCmdStr=${findCmdStr}${beginPrefix}${pathString}${fileString}${endPrefix}"-type f -print"
echo "*******************************************************************************"
echo "CSPREFIX     : $csprefix"
echo "COORDSVCNAME : $coordsvcname"
echo "Find command : $findCmdStr"
echo "*******************************************************************************"

#for file in $($findCmdStr)
#do
#   echo ${file}
#done

# begin to test...
echo ""
echo -e "\e[46;31m ======>Begin to test usecase=====> \e[0m"
echo ""

libJSStr=""
postfix=""
testFile=""
beginTime=`date`
beginTimeSec=`date +%s`
for file in $($findCmdStr)
do
   postfix="${file##*.}"
   if [ "$postfix" != "js" ] ; then
      continue
   fi

   libJSStr="${file%/*}"
   libJSStr=${libJSStr}"lib.js"
   if [ -e $libJSStr ] ; then
      testFile=${libJSStr}","${file}
   else
      testFile=${file}
   fi

   echo "===>[$file]"
   $sdbRoot/sdb -s "db = new Sdb('localhost','${coordsvcname}' ) ; db.msg('Begin test[$file]') ; "
   $sdbRoot/sdb -e "var CSPREFIX='${csprefix}'; var COORDSVCNAME=${coordsvcname}" -f "testcases/hlt/js_testcases/libs/func.js,$testFile"
   ret=$?
   $sdbRoot/sdb -s "db = new Sdb('localhost',${coordsvcname} ) ; db.msg('End test[$file]') ; "
   echo -n "<===[$file]"
   if [ $ret -ne 0 ]
   then
      failedNum=`expr $failedNum + 1`
      echo -e "\033[31;49;1m [ Failed:$failedNum ] \033[39;49;0m"
      if [ $stopWhenFailed -ne 0 ] ; then
         break
      fi
   else
      sucNum=`expr $sucNum + 1`
      echo -e "\033[32;49;1m [ Done:$sucNum ] \033[39;49;0m"
   fi
   echo ""
done
endTime=`date`
endTimeSec=`date +%s`

echo -e "\e[46;31m <======End test usecase<===== \e[0m"
showResult
exit 0

