#bin/bash

# define root path
#testRoot="testcases/hlt/basic_testcases/js"
testRoot="testcases/hlt/js_testcases/js"
libRoot="testcases/hlt/js_testcases/libs"
#libRoot="testcases/hlt/basic_testcases/libs"
sdbRoot="bin"
csprefix="local_test"
uuid=$$
uuname="s$$test"
coordsvcname="50000"
catasvcname="30000"
coordhostname="localhost"
spareportstart="26000"
spareportstop="27000"
runresult=0
commlibstr="commlib.js"
reportDir=${csprefix}"_report"
passDir=""
passFile=""

# define test parameter
testDir=$testRoot
testFile=""
stopWhenFailed=1
printOut=0
showNameWidth=60
runAllTest=0
specificDirorFile=0

# define stat parameter
sucNum=0
failedNum=0
useTime=0
beginTime=0
endTime=0
beginTimeSec=0
endTimeSec=0

testcaseBTimeSec=0
testcaseETimeSec=0

printStr=""
lastCmdStr=""
needExit=0
# define ignore path and file
pathArray=("vote")
fileArray=("commlib.js")

# common function
function display()
{
   echo "run testcase 1.0.0 2014/2/25"
   echo "$0 --help"
   echo "$0 [-p path]|[-f file] [-s stopFlag] [-n svcname] [-h hostname] [-s1] [-s2] [-addpid] [-print] [-all]"
   echo ""
   echo " -p path     : 运行指定路径下的JS用例，如果为相对目录，则默认根目录已为用例目录"
   echo " -f file     : 运行指定的JS用例，如果为相对目录，则默认根目录已为用例目录"
   echo " -s stopFlag : 发生用例错误是否停止，0表示继续，1表示停止"
   echo " -n svcname  : 指定测试的COORD节点服务名"
   echo " -h hostname : 指定测试的COORD节点HostName或IP"
	echo " -s1         : 指定预留的SPAREPORTSTART端口号"
   echo " -s2         : 指定预留的SPAREPORTSTOP端口号"
   echo " -c cataport : 指定测试的CATALOG节点服务名"
   echo " -addpid     : 是否在CHANGEDPREFIX上加上当前进行PID"
   echo " -print      : 是否在屏幕上打印用例的输出"
   echo " -all        : 是否跑所有的测试用例.默认跑基本测试用例  "
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

function printResult()
{
   echo "$1" >> ${reportDir}/result.txt
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
   if [ $1 -ne 0 ] ; then
      echo -e "\033[32;49;1m $sucNum \033[39;49;0m"
   else
      echo " $sucNum"
   fi
   echo -n " failed    :"
   if [ $failedNum -ne 0 -a $1 -ne 0 ] ; then
      echo -e "\033[31;49;1m $failedNum \033[39;49;0m"
   else
      echo " $failedNum"
   fi
   echo "***********************************************************"
}

function prepareRun()
{
   # remove result
   if [ -d $reportDir ] ; then
      rm -rf $reportDir/*
   else
      mkdir ${reportDir}
   fi
}

function runJSFile()
{
   result=0 ;
   lastCmdStr="$sdbRoot/sdb -e \"var CHANGEDPREFIX='${csprefix}'; var COORDSVCNAME='${coordsvcname}'; var COORDHOSTNAME='${coordhostname}';var SPAREPORTSTART='${spareportstart}';var SPAREPORTSTOP='${spareportstop}'; var CATASVCNAME='$catasvcname'; var UUID=$uuid; var UUNAME='${uuname}'; var RUNRESULT=$runresult; \" -f \"${libRoot}/func.js,$1\""
   runresult=0
   if [ $printOut -ne 0 -o $# -gt 1 ] ; then
      echo "CMD: $lastCmdStr"
      eval $lastCmdStr
      result=$?
   else
      if [ ! -d $shortDir ] ; then
         mkdir -p $shortDir
      fi
      echo "CMD: $lastCmdStr" >> ${printOutFile}
      eval $lastCmdStr >> ${printOutFile}
      result=$?
   fi
   return $result ;
}

function procJSFile()
{
   file=$1
   shortFile="${file#$testRoot/}"
   shortDir="${shortFile%/*}"
   if [ "${shortDir:0:1}" == "/" ] ; then
      shortDir=""
   fi
   shortDir=${reportDir}"/"${shortDir}
   printOutFile=${reportDir}"/"${shortFile}"_out.txt"

   postfix="${file##*.}"
   if [ "$postfix" != "js" ] ; then
      return 1
   fi


   libJSStr="${file%/*}"
   libJSStr=${libJSStr}/${commlibstr}
   if [ -e $libJSStr ] ; then
         testFile=${libJSStr}","${file}
   else
         testFile=${file}
   fi

   if [ $printOut -ne 0 ] ; then
      echo "===>[$shortFile]"
   else
      #echo -n "$shortFile   "
      printf "===> %-${showNameWidth}s" $shortFile
   fi

   # run prepare for testcase
   runJSFile "${libRoot}/before_usecase.js"

   testcaseBTimeSec=`date +%s`
   $sdbRoot/sdb -s "try{ db.msg('Begin test[$file]') ; } catch( e ) { } "
   runJSFile "$testFile"
   ret=$?
   runresult=$ret
   $sdbRoot/sdb -s "try{ db.msg('End test[$file]') ; } catch( e ) {} "
   testcaseETimeSec=`date +%s`
   if [ $printOut -ne 0 ] ; then
      echo -n "<===[$shortFile]"
   fi
   if [ $ret -ne 0 ]
   then
      failedNum=`expr $failedNum + 1`
      #printResult "$shortFile --- [ Failed ] `expr $testcaseETimeSec - $testcaseBTimeSec`(s)"
      printResult "$(printf "===> %-${showNameWidth}s" $shortFile) [ Failed ] `expr $testcaseETimeSec - $testcaseBTimeSec`(s)"
      echo -e "\033[31;49;1m [ Failed:$failedNum ] `expr $testcaseETimeSec - $testcaseBTimeSec`(s) \033[39;49;0m"
   else
      sucNum=`expr $sucNum + 1`
      #printResult "$shortFile --- [ Done ] `expr $testcaseETimeSec - $testcaseBTimeSec`(s)"
      printResult "$(printf "===> %-${showNameWidth}s" $shortFile) [ Done ] `expr $testcaseETimeSec - $testcaseBTimeSec`(s)"
      echo -e "\033[32;49;1m [ Done:$sucNum ] `expr $testcaseETimeSec - $testcaseBTimeSec`(s) \033[39;49;0m"
   fi

   # run clear for testcase
   runJSFile "${libRoot}/after_usecase.js"

   if [ $ret -ne 0 -a $stopWhenFailed -ne 0 ] ; then
      return 2
   fi

   if [ $printOut -ne 0 ] ; then
      echo ""
   fi
   return 0
}

function procBasicTestCase()
{
   if [ $needExit -eq 1 ]
   then
      return
   fi
   if [ -d "$1" ];then
      for dir in $pathArray
      do
         if [ "$1" == "$testRoot/$dir" ]
         then
            return;
         fi
      done
      
      if [  -f "$1/basic_testcases.list" ]
      then
         IFS=,
         arr=(`cat $1/basic_testcases.list |awk -F '=' '{print $2}'`)
         IFS=
         for item in ${arr[*]}
         do
            if [ -f "$1/$item" ]
            then
               procJSFile "$1/$item"
               retCode=$?
               if [ $retCode -eq 2 ]
               then
                 needExit=1
                 break; 
               fi
            fi
         done
         return;
      fi

      #if [ -f "$1/basic.txt" ];then
      #   while read line
      #   do
      #      if [ -f "$1/$line" ];then
      #         procJSFile "$1/$line"
      #      fi
      #   done < "$1/basic.txt"
      #   return
      #fi

      for cur in `ls -l $1 |awk '{print $9}'`
      do
         procBasicTestCase "$1/$cur";
      done
   else
      #procJSFile $1
      return;
   fi
}

# ***************************************************************
# run entry
# ***************************************************************
if [ $# -eq 1 -a "$1" = "--help" ] ; then
   display 0
fi

# loop all parameter
#p=""
#readType=0 # 1: path, 2: file, 3: stopWhenFailed, 4: svcname, 5: hostname

while [ "$1" != "" ]; do
   case $1 in
      -p )            shift
                      specificDirorFile=1
                      passDir=$1
                      ;;
      -f )            shift
                      specificDirorFile=1
                      passFile=$1
                      ;;
      -s )            shift
                      stopWhenFailed=$(($1))
                      ;;
      -n )            shift
                      coordsvcname="$1"
                      ;;
      -h )            shift
                      coordhostname="$1"
                      ;;
      -s1)            shift
                      spareportstart="$1"
                      ;;
      -s2)            shift
                      spareportstop="$1"
                      ;;
      -c )            shift
                      catasvcname="$1"
                      ;;
      -print )        printOut=1
                      ;;
      -addpid )       csprefix="local_para_$$"
                      reportDir=${csprefix}"_report"
                      ;;
      -all )          #testRoot="testcases/hlt/js_testcases/js"
                      #libRoot="testcases/hlt/js_testcases/libs"
                      #testDir=$testRoot
                      runAllTest=1
                      ;;
      * )             echo "invalid arguments: $1"
                      display 1
                      ;;
   esac
   shift
done
if [ ""X != "$passDir"X ]; then
   testDir="$(getMyPath $passDir)"
fi
echo "after "$testDir
if [ ""X != "$passFile"X ]; then
   testFile="$(getMyPath $passFile)"
fi
echo $testFile
#for p in $@
#do
#   if [ $readType -eq 1 ] ; then
#      testDir="$(getMyPath $p)"
#      readType=0
#   elif [ $readType -eq 2 ] ; then
#      testFile="$(getMyPath $p)"
#      readType=0
#   elif [ $readType -eq 3 ] ; then
#      stopWhenFailed=$(($p))
#      readType=0
#   elif [ $readType -eq 4 ] ; then
#      coordsvcname="$p"
#      readType=0
#   elif [ $readType -eq 5 ] ; then
#      coordhostname="$p"
#      readType=5
#   elif [ "$p" = "-p" ] ; then
#      readType=1 ;
#   elif [ "$p" = "-f" ] ; then
#      readType=2
#   elif [ "$p" = "-s" ] ; then
#      readType=3
#   elif [ "$p" = "-n" ] ; then
#      readType=4
#   elif [ "$p" = "-h" ] ; then
#      readType=5
#   elif [ "$p" = "-addpid" ] ; then
#      csprefix="local_para_$$"
#      reportDir=${csprefix}"_report"
#   elif [ "$p" == "-print" ] ; then
#      printOut=1
#   else
#      echo "invalid arguments: $p"
#      display 1
#   fi
#done

#if [ $readType -ne 0 ] ; then
#   echo "invalid arguments"
#   display 1
#fi

if [ "$testFile" != "" ] ; then
   testDir="$testFile"
   unset fileArray
fi

if [ "$testDir" != "$testRoot" ] ; then
   unset pathArray
fi

# construct exclude dirs and exclude files
pathString=""
fileString=""
findCmdStr="find $testDir "
beginPrefix=""
endPrefix=""

if [ $runAllTest -eq 1 -o $specificDirorFile -eq 1 ]
then
   for data in ${pathArray[@]}
   do
      if [ "$pathString" != "" ] ; then
         pathString=${pathString}" -o "
      fi
      pathString=${pathString}"-path ""\""*/${data}"\""
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
   if [ "$pathString" != "" -o "$fileString" != "" ] ; then
      findCmdStr=${findCmdStr}${beginPrefix}"\( "${pathString}${fileString}" \)"${endPrefix}"-type f -print"
   else
      findCmdStr=${findCmdStr}${beginPrefix}${endPrefix}"-type f -print"
   fi
   echo "*******************************************************************************"
   echo "CHANGEDPREFIX : $csprefix"
   echo "UUID          : $uuid"
   echo "UUNAME        : $uuname"
   echo "COORDSVCNAME  : $coordsvcname"
   echo "COORDSVCHOST  : $coordhostname"
   echo "SPAREPORTSTART: $spareportstart"
   echo "SPAREPORTSTOP : $spareportstop"
   echo "Find command  : $findCmdStr"
   echo "*******************************************************************************"
else
   echo "*******************************************************************************"
   echo "CHANGEDPREFIX : $csprefix"
   echo "UUID          : $uuid"
   echo "UUNAME        : $uuname"
   echo "COORDSVCNAME  : $coordsvcname"
   echo "COORDSVCHOST  : $coordhostname"
   echo "SPAREPORTSTART: $spareportstart"
   echo "SPAREPORTSTOP : $spareportstop"
   echo "Exec command  : ls $testDir/*/basic_testcases.list" 
   echo "*******************************************************************************"
fi

#for file in `eval $findCmdStr`
#do
#   echo ${file}
#done
#exit 0

# begin to test...
echo ""
echo -e "\e[46;31m ======>Begin to test usecase=====> \e[0m"
echo ""

# prepare for running
prepareRun

# create msg db connection
$sdbRoot/sdb -s "try { var db = new Sdb('${coordhostname}', '${coordsvcname}' ) } catch( e ) {} "

libJSStr=""
postfix=""
testFile=""
shortFile=""
printOutFile=""
shortDir=""
beginTime=`date`
beginTimeSec=`date +%s`

# before all test-cases running
printStr="$(runJSFile "${libRoot}/all_prepare.js" 0 )"
if [ "$printStr" != "" ] ; then
   printResult "+++++++++++++++++++++++++++++++++++++++++++++++"
   printResult "$printStr"
   printResult "-----------------------------------------------"
   printResult ""
fi

if [ $runAllTest -eq 1 -o $specificDirorFile -eq 1 ]
then
   for file in `eval $findCmdStr`
   do
      procJSFile $file
      retCode=$?
      if [ $retCode -eq 1 ]
      then
         continue
      elif [ $retCode -eq 2 ]
      then
         break
      fi      
   done
else
   procBasicTestCase $testDir
fi

endTime=`date`
endTimeSec=`date +%s`

# after all test-cases clear
printStr="$(runJSFile "${libRoot}/all_clean.js" 0)"
if [ "$printStr" != "" ] ; then
   printResult ""
   printResult "++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
   printResult "$printStr"
   printResult "--------------------------------------------------------"
fi

# destory db connection
$sdbRoot/sdb -s "try { db.close() ; } catch( e ) {} "
echo -e "\e[46;31m <======End test usecase<===== \e[0m"

# show result screen
showResult 1
# show result file
printStr="$(showResult 0 )"
printResult ""
printResult "$printStr"

# quit
exit 0

