#!/bin/bash

# define test root path
storyTestRoot="testcase_new/story/js"
sdvTestRoot="testcase_new/sdv/js"
storyATTestRoot="testcase_new/story_at/js"
testRoots=($storyTestRoot)

libRoot="testcase_new/story/js/lib"
commlibstr="commlib.js"

sdbRoot="bin"

coordsvcname="50000"
essvcname="9200"
dssvcname="11810"
catasvcname="30000"
coordhostname="localhost"
eshostname="localhost"
dshostname="localhost"

rsrvportbegin="26000"
rsrvportend="27000"
runtest_path=`pwd`
rsrvnodedir="${runtest_path}/database_runtest/"

# only for all_clean
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
testType="basic"
runAllTest=0
specificDirorFile=0
remoteuser=sdbadmin
remotepasswd=Admin@1024

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
   echo "$0 [-p path]|[-f file] [-t type] [-s stopFlag] [-n svcname] [-h hostname] [--user user] [--password password] [-eh eshost] [-en essvcname] [-dh dshost] [-dn dssvcname] [-s1] [-s2] [-sp] [-addpid] [-print]"
   echo ""
   echo " -p path        : 运行指定路径下的JS用例。为相对目录，默认根目录为用例目录"
   echo " -f file        : 运行指定的JS用例。为相对目录，默认根目录为用例目录"
   echo -e " -t type        : 运行指定类型的用例，可取story|story_at|sdv|dev|all\n"\
           "                  - story：功能测试用例；story_at：功能验收用例；sdv：系统设计验证用例\n"\
           "                  - 指定 dev 会运行 story 及 story_at 用例，指定 all 会运行 story、story_at 及 sdv 用例\n"\
           "                  - 当不指定-t-p-f时，默认跑基本用例；当不指定-t指定了-f|-p默认跑story"
   echo " -s stopFlag    : 发生用例错误是否停止，0表示继续，1表示停止，默认为1"
   echo " -n svcname     : 指定测试的COORD节点服务名，默认为50000"
   echo " -h hostname    : 指定测试的COORD节点HostName或IP"
   echo " --user         : 远程机器用户名, 默认为sdbadmin"
   echo " --password     : 远程机器用户密码, 默认为Admin@1024"
   echo " -c cataport    : 指定测试的CATALOG节点服务名，默认为30000"
   echo " -eh eshost     : 指定es环境主机名或ip，默认是localhost"
   echo " -en essvcname  : 指定es环境节点服务名，默认为9200"
   echo " -dh dshost     : 指定数据源主机名或ip，默认是localhost"
   echo " -dn dssvcname  : 指定数据源节点服务名，默认是11810"
   echo " -s1            : 指定预留的RSRVPORTBEGIN端口号，默认为26000"
   echo " -s2            : 指定预留的RSRVPORTEND端口号，默认为27000"
   echo " -sp            : 指定用预留端口创建节点的路径RSRVNODEDIR，默认为 当前路径/database_runtest/"
   echo " -addpid        : 是否在CHANGEDPREFIX上加上当前进行PID"
   echo " -print         : 是否在屏幕上打印用例的输出"
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
   lastCmdStr="$sdbRoot/sdb -e \"var CHANGEDPREFIX='${csprefix}'; var COORDSVCNAME='${coordsvcname}'; var COORDHOSTNAME='${coordhostname}';var REMOTEUSER='${remoteuser}';var REMOTEPASSWD='${remotepasswd}';var ESSVCNAME='${essvcname}'; var ESHOSTNAME='${eshostname}';var DSSVCNAME='${dssvcname}'; var DSHOSTNAME='${dshostname}';var RSRVPORTBEGIN='${rsrvportbegin}';var RSRVPORTEND='${rsrvportend}'; var CATASVCNAME='$catasvcname'; var RSRVNODEDIR='$rsrvnodedir'; var RUNRESULT=$runresult; \" -f \"${libRoot}/func.js,$file\""
   if [[ $file == *"charset_gb18030"* ]]; then
      lastCmdStr+=" -c GB18030"
   fi
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

   if [ $printOut -eq 1 ] ; then
      echo "===>[$shortFile]"
   else
      #echo -n "$shortFile   "
      printf "===> %-${showNameWidth}s" $shortFile
   fi

   # run prepare for testcase
   #runJSFile "${libRoot}/before_usecase.js"

   testcaseBTimeSec=`date +%s`
   $sdbRoot/sdb -s "try{ db.msg('Begin testcase[$file]') ; } catch( e ) { } "
   runJSFile "$testFile"
   ret=$?
   # ret == 0 ? runresult : ret
   runresult=$([ $ret == 0 ] && echo "${runresult}" || echo "${ret}" )
   $sdbRoot/sdb -s "try{ db.msg('End testcase[$file]') ; } catch( e ) {} "
   testcaseETimeSec=`date +%s`
   if [ $printOut -eq 1 ] ; then
      echo -n "<===[$shortFile]"
   fi

   if [ $ret -ne 0 ]
   then
      failedNum=`expr $failedNum + 1`
      #printToResultFile "$shortFile --- [ Failed ] `expr $testcaseETimeSec - $testcaseBTimeSec`(s)"
      printToResultFile "$(printf "===> %-${showNameWidth}s" $shortFile) [ Failed ] `expr $testcaseETimeSec - $testcaseBTimeSec`(s)"
      echo -e "\033[31;49;1m [ Failed:$failedNum ] `expr $testcaseETimeSec - $testcaseBTimeSec`(s) \033[39;49;0m"
   else
      sucNum=`expr $sucNum + 1`
      #printToResultFile "$shortFile --- [ Done ] `expr $testcaseETimeSec - $testcaseBTimeSec`(s)"
      printToResultFile "$(printf "===> %-${showNameWidth}s" $shortFile) [ Done ] `expr $testcaseETimeSec - $testcaseBTimeSec`(s)"
      echo -e "\033[32;49;1m [ Done:$sucNum ] `expr $testcaseETimeSec - $testcaseBTimeSec`(s) \033[39;49;0m"
   fi

   # run clear for testcase
   if [ $ret -ne 0 -a $stopWhenFailed -ne 0 ] ; then
   #  runresult=$ret
      #runJSFile "${libRoot}/after_usecase.js"
      return 2
   fi

   #  runresult=0
   #runJSFile "${libRoot}/after_usecase.js"

   if [ $printOut -eq 1 ] ; then
      echo ""
   fi

   return 0
}

# process basic testcases
function procBasicTestCase()
{
   local testRoot=$2

   if [ $needExit -eq 1 ]
   then
      return
   fi
   if [ -d "$1" ];then
      for dir in $ignoredPaths
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
               procJSFile "$1/$item" $testRoot
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
         procBasicTestCase "$1/$cur" $testRoot
      done
   else
      #procJSFile $1
      return;
   fi
}

#analysis parameters
#函数参数为：脚本参数
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
         --user)         shift
                         remoteuser="$1"
                         ;;
         --password)     shift
                         remotepasswd="$1"
                         ;;
         -sp )           shift
                         rsrvnodedir="$1"
                         ;;
         -t )            shift
                         testType="$1"
                         ;;
         -eh )           shift
                         eshostname="$1"
                         ;;
         -en )           shift
                         essvcname="$1"
                         ;;
         -dh )           shift
                         dshostname="$1"
                         ;;
         -dn )           shift
                         dssvcname="$1"
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
               runAllTest=1
               ;;
      sdv )    testRoots[0]=$sdvTestRoot
               runAllTest=1
               ;;
      story_at )testRoots[0]=$storyATTestRoot
               runAllTest=1
               ;;
      dev )    testRoots[0]=$storyTestRoot
               testRoots[1]=$storyATTestRoot
               runAllTest=1
               ;;
      all )    testRoots[0]=$storyTestRoot
               testRoots[1]=$sdvTestRoot
               testRoots[2]=$storyATTestRoot
               runAllTest=1
               ;;
      basic )
               runAllTest=0
               ;;
      * )      echo "invalid testType: $testType"
               showHelpInfo 1
               ;;
   esac
}

function filterTestcase()
{
   local testRoot=$1
   pathLists=(`sed -n '2,7p' $testRoot/testcase.conf |awk -F '=' '{print $2}'`)
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

   if [ $runAllTest -eq 1 -o $specificDirorFile -eq 1 ]
   then
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
   else #basic
      echo "Exec command  : ls $testDir/*/basic_testcases.list"
   fi
}

# remove all reports in "local_test_report"
function removeReport()
{
   if [ -d $reportDirRoot ] ; then
      rm -rf $reportDirRoot/
   fi
   mkdir ${reportDirRoot}
}

function mainRun()
{
   local findCmdStr=$1
   local testRoot=$2
   local beforeModuleFileName="before_module.js"
   local afterModuleFileName="after_module.js"
   local moduleReportDir=""

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

   local lastPath=""
   local needModuleCleanup=false
   # run all test-cases
   if [ $runAllTest -eq 1 -o $specificDirorFile -eq 1 ]
   then
      for file in `eval $findCmdStr | sort`
      do
         # Skip the module prepare/cleanup files.
         fileName=`basename $file`
         if [ $fileName == $beforeModuleFileName ] || [ $fileName == $afterModuleFileName ]
         then
            continue
         fi
         testPath=`dirname $file`
         # Do module prepare when entering a new test directory
         if [ "$testPath" != "$lastPath" ]
         then
            # Do cleanup of last module
            if [ -n "$lastPath" ] && [ -f "$afterModuleFile" ]
            then
               printOutFile=${moduleReportDir}"/after_module_out.txt"
               runJSFile "${afterModuleFile}"
               needModuleCleanup=false
            fi

            parentDir=`basename $testPath`
            moduleReportDir=${reportDir}"/"${parentDir}
            beforeModuleFile=${testPath}"/"${beforeModuleFileName}
            afterModuleFile=${testPath}"/"${afterModuleFileName}
            # Create the module report directory if it dose not exist
            if [ ! -d "$moduleReportDir" ]
            then
               mkdir ${moduleReportDir}
            fi

            if [ -f $beforeModuleFile ]
            then
               printOutFile="${moduleReportDir}/before_module_out.txt"
               runJSFile "${beforeModuleFile}"
               needModuleCleanup=true
            fi
            lastPath=$testPath
         fi

         procJSFile $file $testRoot
         retCode=$?
         if [ $retCode -eq 1 ]
         then
            continue
         elif [ $retCode -eq 2 ]
         then
            break
         fi
      done

      if [ "$needModuleCleanup" = true ] && [ -n "$lastPath" ]
      then
         # Do cleanup of last module
         afterModuleFile=${lastPath}"/"${afterModuleFileName}
         if [ -f "$afterModuleFile" ]
         then
            printOutFile=${moduleReportDir}"/after_module_out.txt"
            runJSFile "${afterModuleFile}"
            needModuleCleanup=false
         fi
      fi
   else
      procBasicTestCase $testDir $testRoot
   fi


   # after all test-cases clear
   printStr="$(runJSFile "${libRoot}/all_clean.js" 0)"
   if [ "$printStr" != "" ] ; then
   printToResultFile ""
   printToResultFile "++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
   printToResultFile "$printStr"
   printToResultFile "--------------------------------------------------------"
   fi

   # destory db connection
   $sdbRoot/sdb -s "try { db.close() ; } catch( e ) {} "
}

# ***************************************************************
#                       run entry: main
# ***************************************************************

analyPara $*

analyTestType

removeReport

#print all parameter to screen
echo ""
echo "**************************************************************************************"
echo "CHANGEDPREFIX : $csprefix"
echo "COORDSVCNAME  : $coordsvcname"
echo "COORDHOSTNAME : $coordhostname"
echo "ESSVCNAME     : $essvcname"
echo "ESHOSTNAME    : $eshostname"
echo "DSSVCNAME     : $dssvcname"
echo "DSHOSTNAME    : $dshostname"
echo "RSRVPORTBEGIN : $rsrvportbegin"
echo "RSRVPORTEND   : $rsrvportend"
echo "RSRVNODEDIR   : $rsrvnodedir"
echo "REMOTEUSER    : $remoteuser"
echo "SDBADMINPWD   : $remotepasswd"

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

for(( i=0; i<${#testRoots[@]}; i++ ))
do
   libRoot="${testRoots[i]}/lib"
   case ${testRoots[i]} in
      $storyTestRoot )  reportDir="${reportDirRoot}/story"
                        tmpType="story"
                        mkdir ${reportDir}
                        ;;
      $sdvTestRoot )    reportDir="${reportDirRoot}/sdv"
                        tmpType="sdv"
                        mkdir ${reportDir}
                        ;;
      $storyATTestRoot )reportDir="${reportDirRoot}/story_at"
                        tmpType="story_at"
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
