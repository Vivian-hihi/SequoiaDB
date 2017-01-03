#!/bin/bash

# define test root path
storyTestRoot="testcase_new/story/js"
sdvTestRoot="testcase_new/sdv/js"
testRoots=($storyTestRoot)

libRoot="testcase_new/story/js/lib"
commlibstr="commlib.js"

sdbRoot="bin"
uuid=$$
uuname="s$$test"

coordsvcname="50000"
catasvcname="30000"
coordhostname="localhost"

rsrvportbegin="26000"
rsrvportend="27000"
rsrvnodedir="/opt/sequoiadb/database/"

runresult=0

csprefix="local_test"
reportDirRoot=${csprefix}"_report"

passDir=""
passFile=""

# define test parameter
testFile=""
stopWhenFailed=1
printOut=0
showNameWidth=60
testType="story"
specificDirorFile=0
threadNum=1

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
ignoredPaths=()
#ignoredPaths=("vote" "dataCompress" "bakupRestore")
ignoredFiles=("commlib.js")

# print help information
function showHelpInfo()
{
   echo "run testcase 1.0.0 2014/2/25"
   echo "$0 --help"
   echo "$0 [-p path]|[-f file] [-t type] [-s stopFlag] [-n svcname] [-h hostname] [-s1] [-s2] [-sp] [-addpid] [-print]"
   echo ""
   echo " -p path     : 运行指定路径下的JS用例。为相对目录，默认根目录为用例目录"
   echo " -f file     : 运行指定的JS用例。为相对目录，默认根目录为用例目录"
   echo " -t type     : 运行指定类型的用例，可取story|sdv|all。当不指定-t-p-f时，默认跑基本用例；当不指定-t指定了-f|-p默认跑story"
   echo " -s stopFlag : 发生用例错误是否停止，0表示继续，1表示停止，默认为1"
   echo " -n svcname  : 指定测试的COORD节点服务名"
   echo " -h hostname : 指定测试的COORD节点HostName或IP"
   echo " -c cataport : 指定测试的CATALOG节点服务名"
   echo " -j thnum    : 指定测试的线程数"
   echo " -s1         : 指定预留的RSRVPORTBEGIN端口号，默认为26000"
   echo " -s2         : 指定预留的RSRVPORTEND端口号，默认为27000"
   echo " -sp         : 指定用预留端口创建节点的路径RSRVNODEDIR，默认为/opt/sequoiadb/database/"
   echo " -addpid     : 是否在CHANGEDPREFIX上加上当前进行PID"
   echo " -print      : 是否在屏幕上打印用例的输出"
   echo ""
   exit $1
}

# print content to result.txt
# $1：the content
function printToResultFile() 
{
   echo "$1" >> ${reportDirRoot}/result.txt
}

function showResult()
{
   local flag=$1

   echo "***********************************************************"
   echo "                    ***test result*** "
   echo " num of thd: $threadNum"
   echo " begin time: $beginTime"
   echo " end time  : $endTime"
   echo " use time  : `expr $endTimeSec - $beginTimeSec`(secs)"
   echo " total     : `expr $sucNum + $failedNum`"

   echo -n " succeed   :"
   if [ $flag -ne 0 ] ; then
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

# run a js file
# $1: js file directory
function runJSFile()
{
   local file=$1

   result=0
   lastCmdStr="$sdbRoot/sdb -e \"var CHANGEDPREFIX='${csprefix}'; var COORDSVCNAME='${coordsvcname}'; var COORDHOSTNAME='${coordhostname}';var RSRVPORTBEGIN='${rsrvportbegin}';var RSRVPORTEND='${rsrvportend}'; var CATASVCNAME='$catasvcname'; var RSRVNODEDIR='$rsrvnodedir'; var UUID=$uuid; var UUNAME='${uuname}'; var RUNRESULT=$runresult; \" -f \"${libRoot}/func.js,$file\""
#   runresult=0
   if [ $printOut -eq 1 -o $# -gt 1 ] ; then
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

# process a js testcase
function procJSFile()
{
   local file=$1
   local testRoot=$2

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

   # run prepare for testcase
   runJSFile "${libRoot}/before_usecase.js"

   testcaseBTimeSec=`date +%s`
   $sdbRoot/sdb -s "try{ db.msg('Begin testcase[$file]') ; } catch( e ) { } "
   runJSFile "$testFile"
   ret=$?
#   runresult=$ret
   $sdbRoot/sdb -s "try{ db.msg('End testcase[$file]') ; } catch( e ) {} "
   testcaseETimeSec=`date +%s`

   (
      flock -x -w 10 202 || exit 1
      printf "===> %-${showNameWidth}s" $shortFile
      if [ $ret -ne 0 ]
      then
         failedNum=`cat $reportDirRoot/.failed.log`
         failedNum=`expr $failedNum + 1`
         echo $failedNum>$reportDirRoot/.failed.log
         printToResultFile "$(printf "===> %-${showNameWidth}s" $shortFile) [ Failed ] `expr $testcaseETimeSec - $testcaseBTimeSec`(s)"
         echo -e "\033[31;49;1m [ TID: $tid Failed: $failedNum Timd: `expr $testcaseETimeSec - $testcaseBTimeSec`(s) ] \033[39;49;0m"
      else
         sucNum=`cat $reportDirRoot/.succeed.log`
         sucNum=`expr $sucNum + 1`
         echo $sucNum>$reportDirRoot/.succeed.log
         printToResultFile "$(printf "===> %-${showNameWidth}s" $shortFile) [ Done ] `expr $testcaseETimeSec - $testcaseBTimeSec`(s)"
         echo -e "\033[32;49;1m [ TID: $tid Done: $sucNum Time: `expr $testcaseETimeSec - $testcaseBTimeSec`(s) ] \033[39;49;0m"
      fi
   )202>$reportDirRoot/.test.lock

   # run clear for testcase
   if [ $ret -ne 0 -a $stopWhenFailed -ne 0 ] ; then
      runresult=$ret
      runJSFile "${libRoot}/after_usecase.js"
      return 2
   fi

   runresult=0
   runJSFile "${libRoot}/after_usecase.js"

   if [ $printOut -eq 1 ] ; then
      echo ""
   fi

   return 0
}

#analysis parameters
function analyPara()
{
   if [ $# -eq 1 -a "$1" = "--help" ] ; then
   showHelpInfo 0
   fi

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
         -c )            shift
                         catasvcname="$1"
                         ;;                
         -s1)            shift
                         rsrvportbegin="$1"
                         ;;
         -s2)            shift
                         rsrvportend="$1"
                         ;;
         -sp )           shift
                         rsrvnodedir="$1"
                         ;;
         -t )            shift
                         testType="$1"
                         ;;
         -j )            shift
                         threadNum="$1"
                         ;;
         -print )        printOut=1
                         ;;
         -addpid )       csprefix="local_para_$$"
                         reportDirRoot=${csprefix}"_report"
                         ;;
         * )             echo "invalid arguments: $1"
                         showHelpInfo 1
                         ;;
      esac
   shift
   done
}

function analyTestType()
{
   case $testType in
      story )  testRoots[0]=$storyTestRoot
               ;;
      sdv )    testRoots[0]=$sdvTestRoot
               ;;
      all )    testRoots[0]=$sdvTestRoot
               testRoots[1]=$storyTestRoot
               ;;
      * )      echo "invalid testType: $testType"
               showHelpInfo 1
               ;;
   esac
}

function filterTestcase()
{
   local testRoot=$1

   pathLists=(`sed -n '2,6p' $testRoot/testcase.conf |awk -F '=' '{print $2}'`)
   for pathList in ${pathLists[@]}
   do
      path2Space=${pathList//,/ }
      for path in $path2Space
      do
         ignoredPaths+=($path)
      done
   done
}

function generateFindCmd()
{
   local testRoot=$1
   testDir=$testRoot

   if [ "$passDir" != "" ]; then
      if [ ${passDir:0:1} = "/" ] ; then
         testDir="$passDir"
      else
         testDir="$testRoot/$passDir"
      fi
   fi

   if [ "$passFile" != "" ]; then
      if [ ${passFile:0:1} = "/" ] ; then
         testFile="$passFile"
      else
         testFile="$testRoot/$passFile"
      fi
   fi

   if [ "$testFile" != "" ] ; then
      testDir="$testFile"
      unset ignoredFiles
   fi

   if [ "$testDir" != "$testRoot" ] ; then
      unset ignoredPaths
   fi

   # construct exclude dirs and exclude files
   pathString=""
   fileString=""
   findCmdStr="find $testDir "
   beginPrefix=""
   endPrefix=""

   for data in ${ignoredPaths[@]}
   do
      if [ "$pathString" != "" ] ; then
         pathString=${pathString}" -o "
      fi
      pathString=${pathString}"-path ""\""*/${data}"\""
      beginPrefix=" "
      endPrefix=" -prune -o  "
   done

   for data in ${ignoredFiles[@]}
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

   echo "Find command  : $findCmdStr"
}

# remove all reports in "local_test_report"
function removeReport()
{
   if [ -d $reportDirRoot ] ; then
      rm -rf $reportDirRoot/*
   else
      mkdir ${reportDirRoot}
   fi
}


function runTest()
{
   local tid=$1
   local findCmdStr=$2
   local testRoot=$3
   lastDir="test"
   localDirId=0
   expectedDirId=0

   csprefix="${csprefix}_$tid"

   echo "0">$reportDirRoot/.test.$tid.status

   for file in `eval $findCmdStr | sort`
   do
      curDir=`dirname $file`
      if [ $lastDir != $curDir ]
      then
         lastDir=$curDir
         localDirId=`expr $localDirId + 1`
         (
            flock -x -w 10 203 || exit 1
            globalDirId=`cat $reportDirRoot/.test.status`
            if [ $globalDirId -ne -1 ]
            then
               if [ $localDirId -eq $globalDirId ]
               then
                  globalDirId=`expr $globalDirId + 1`
                  echo $globalDirId>$reportDirRoot/.test.status
                  echo $localDirId>$reportDirRoot/.test.$tid.status
                  printf "===> %-${showNameWidth}s" "[ $tmpType: `basename $lastDir` ]"
                  echo -e "\033[33;49;1m [ TID: $tid Prefix: $csprefix Start ] \033[39;49;0m"
               fi
            else
               echo "-1">$reportDirRoot/.test.$tid.status
            fi
         )203>$reportDirRoot/.test.lock
      fi

      globalDirId=`cat $reportDirRoot/.test.status`
      expectedDirId=`cat $reportDirRoot/.test.$tid.status`
      if [ $expectedDirId -eq -1 -o $globalDirId -eq -1 ]
      then
         break
      fi

      if [ $expectedDirId -ne $localDirId ]
      then
         continue
      fi

      # process JS test files
      procJSFile $file $testRoot
      retCode=$?
      if [ $retCode -eq 2 ]
      then
         (
            flock -x -w 10 204 || exit 1
            echo "-1">$reportDirRoot/.test.status
         )204>$reportDirRoot/.test.lock
         break
      fi
   done

   (
      flock -x -w 10 205 || exit 1
      printf "===> %-${showNameWidth}s" ""
      echo -e "\033[33;49;1m [ TID: $tid End ] \033[39;49;0m"
   )205>$reportDirRoot/.test.lock
}

function cleanUp()
{
   echo "-1">$reportDirRoot/.test.status
   exit 1
}

function mainRun()
{
   local findCmdStr=$1
   local testRoot=$2

   libJSStr=""
   postfix=""
   testFile=""
   shortFile=""
   printOutFile=""
   shortDir=""

   # create msg db connection
   $sdbRoot/sdb -s "try { var db = new Sdb('${coordhostname}', '${coordsvcname}' ) } catch( e ) {} "

   # before all test-cases running
   printStr="$(runJSFile "${libRoot}/all_prepare.js" 0 )"
   if [ "$printStr" != "" ] ; then
   printToResultFile "+++++++++++++++++++++++++++++++++++++++++++++++"
   printToResultFile "$printStr"
   printToResultFile "-----------------------------------------------"
   printToResultFile ""
   fi

   echo "0">$reportDirRoot/.succeed.log
   echo "0">$reportDirRoot/.failed.log
   echo "1">$reportDirRoot/.test.status

   # run all test-cases
   for tid in `seq $threadNum`
   do
      runTest $tid "$findCmdStr" $testRoot &
      testPid[$testTid]=$!
   done

   for tid in `seq $threadNum`;
   do
      wait ${testPid[$tid]}
   done

   # after all test-cases clear
   printStr="$(runJSFile "${libRoot}/all_clean.js" 0)"
   if [ "$printStr" != "" ] ; then
   printToResultFile ""
   printToResultFile "++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
   printToResultFile "$printStr"
   printToResultFile "--------------------------------------------------------"
   fi

   # collect test stats
   localSucNum=`cat $reportDirRoot/.succeed.log`
   localFailedNum=`cat $reportDirRoot/.failed.log`

   sucNum=`expr $sucNum + $localSucNum`
   failedNum=`expr $failedNum + $localFailedNum`

   # destory db connection
   $sdbRoot/sdb -s "try { db.close() ; } catch( e ) {} "
}

# ***************************************************************
#                       run entry
# ***************************************************************

analyPara $*

analyTestType

removeReport

#print all parameter to screen
echo ""
echo "**************************************************************************************"
echo "CHANGEDPREFIX : $csprefix"
echo "UUID          : $uuid"
echo "UUNAME        : $uuname"
echo "COORDSVCNAME  : $coordsvcname"
echo "COORDHOSTNAME : $coordhostname"
echo "SPAREPORTSTART: $spareportstart"
echo "SPAREPORTSTOP : $spareportstop"
echo "SPAREPORTPATH : $spareportPath"

# generate command of find test files, and print
declare -a findCmds                         #define findCmds as array
for testRoot in ${testRoots[@]}
do
   unset ignoredPaths
   filterTestcase $testRoot
   generateFindCmd $testRoot
   findCmds[${#findCmds[@]}]="$findCmdStr"  #add element in tail of array
done
echo "**************************************************************************************"

# get test files and run
beginTime=`date`
beginTimeSec=`date +%s`

trap cleanUp SIGINT

for(( i=0; i<${#testRoots[@]}; i++ ))
do
   case ${testRoots[i]} in
      $storyTestRoot )  reportDir="${reportDirRoot}/story"
                        tmpType="story"
                        mkdir ${reportDir}
                        ;;
      $sdvTestRoot )    reportDir="${reportDirRoot}/sdv"
                        tmpType="sdv"
                        mkdir ${reportDir}
                        ;;
   esac
   echo -e "\e[46;31m ======>Begin to test $tmpType   =====> \e[0m"      #print bule font

   mainRun "${findCmds[i]}" ${testRoots[i]}
done

endTime=`date`
endTimeSec=`date +%s`
echo -e "\e[46;31m <======Finish test all testcases<===== \e[0m"

# show result to screen and file "result.txt"
showResult 1
printToResultFile ""
printToResultFile "$(showResult 0)"

exit 0
