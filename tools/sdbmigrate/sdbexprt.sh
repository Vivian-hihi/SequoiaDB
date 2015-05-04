#!/bin/bash

CUR_PATH=$(cd `dirname $0`; pwd)

# import common functions
source ${CUR_PATH}/common.sh

SDBEXPRT=`getProgFullPath "sdbexprt"`
PROG_PATH=`dirname ${SDBEXPRT}`
SDB=${PROG_PATH}/sdb
SDBEXPRT_LOG="sdbexport.log"
SDBEXPRT_SH_LOG="sdbexport.sh.log"
DEFAULT_TYPE="csv"

opt_version="--version"
opt_help="--help"

opt_hostname=""
opt_svcname=""
opt_user=""
opt_password=""
opt_delchar=""
opt_delfield=""
opt_delrecord=""
opt_csname=""
opt_clname=""
opt_type=""
opt_fields=""
opt_include=""
opt_errorstop=""
opt_includebinary=""
opt_includeregex=""
opt_filter=""
opt_sort=""
opt_prefinst=""
opt_ssl=""
opt_output=""
opt_debug=""

opt_argument_str=""

arr_options=( "-h" "-s" "-p" "-u" "-w" "-a" "-e" "-r" "-c" "-l" "--help" "--version" "--hostname" "--svcname" "--user" "--password" "--delchar" "--delfield" "--delrecord" "--csname" "--clname" "--file" "--type" "--fields" "--include" "--errorstop" "--includebinary" "--includeregex" "--filter" "--sort" "--ssl" "--output" "--debug" )

################################################################################

#
#@description: display usage
#@argument: null
#@return: null
#
function displayUsage()
{
   # filter '-c [ --csname ]' '-l [ --clname ]' '--file'
   $SDBEXPRT "--help" | sed '/-c /d' | sed '/-l /d' | sed '/--file arg/d'
   echo "  --csname arg           specified the cs list to export, use ',' to "
   echo "                         separate the output cs. In general, the output data "
   echo "                         was named by 'cs.cl.type'(e.g. foo.bar.csv), if you "
   echo "                         want to change the cs name, use ':' to specify the new "
   echo "                         cs name. (e.g. --csname 'foo', --csname 'foo:newfoo' "
   echo "                         or --csname 'foo,foo2,foo3:newfoo3')"
   echo "  --clname arg           specified the cl list to export. The rule is the same "
   echo "                         as option '--csname' (e.g. --clname 'foo.bar', "
   echo "                         --clname 'foo.bar:foo.newbar'"
   echo "                         or --clname 'foo.bar,foo1.bar1,foo2.bar2:newfoo2.newbar2'"
   echo "  --output arg           specified the directory where output files are put in"
   echo "  --debug arg            specified the debug level, enum: 1-3, false 1, "
   echo "                         the least information"
}

#
#@description: display version
#@argument: null
#@return: null
#
function displayVersion()
{
   $SDBEXPRT $1
}

#
#@description: verify argument
#@argument: the argument get from command line
#@return: when the argument is ok, return 0, otherwise,
#         stop running
#
function verifyArgument()
{
   local val_opt=$1
   local val_arg=$2
   local elem=""

   if [ "" = "$val_arg" ] ; then
      echo "Error: invalid argument for option '$val_opt'."
      exit
   fi

   for elem in ${arr_options[*]}
   do
      if [ "$val_arg" = "$elem" ] ; then
         echo "Error: invalid argument for option '$val_opt'."
         exit
      fi
   done

   return 0
}

#
#@description: verify the arguments
#@argument: null
#@return: 0 for ok, when error happen, stop running
#
function verifyArgumentsForExport()
{
   # check opt_output
   if [ "" = "$opt_output" ] ; then
      opt_output="."
   fi
   if [ -d $opt_output ] ; then
      :
   else
      echo "Error: the output path '$opt_output' does not exist or it's not a directory."
      exit
   fi
   if [ -w $opt_output ] ; then
      :
   else
      echo "Error: '$opt_output' is not writable."
      exit
   fi

   # change relative path to full path
   opt_output=`getFullPath "$opt_output"`

   # try to set the debug level
   if [ "" = "$opt_debug" ] ; then
      opt_debug="1"
   fi

   # try to set opt_type
   if [ "csv" != "$opt_type" ] && [ "json" != "$opt_type" ] ; then
      opt_type="$DEFAULT_TYPE"
#      echo "Error: option '--type' should be specified as one of the follow: 'csv', 'json'."
#      exit
   fi

   return 0
}

#
#@description: gen the arguments for export one collection
#@argument: cs name and cl name
#@return: the argument string, e.g. 
# --hostname susetzb --svcname 11810 --csname foo1 --clname bar1 --file foo1/foo1.bar1.csv --type csv --fields a,b,c --include true 
#
function genArgForExport()
{
   local arg_cs_name=$1
   local arg_cl_name=$2
   local arg_output_file=$3
   local arg_type=$4
  
   opt_argument_str=""
   opt_argument_str="$opt_argument_str "`genOption "hostname" "${opt_hostname}"`
   opt_argument_str="$opt_argument_str "`genOption "svcname" "${opt_svcname}"`
   opt_argument_str="$opt_argument_str "`genOption "user" "${opt_user}"`
   opt_argument_str="$opt_argument_str "`genOption "password" "${opt_password}"`
   opt_argument_str="$opt_argument_str "`genOption "delchar" "${opt_delchar}"`
   opt_argument_str="$opt_argument_str "`genOption "delfield" "${opt_delfield}"`
   opt_argument_str="$opt_argument_str "`genOption "delrecord" "${opt_delrecord}"`
   opt_argument_str="$opt_argument_str "`genOption "csname" "${arg_cs_name}"` #
   opt_argument_str="$opt_argument_str "`genOption "clname" "${arg_cl_name}"` #
   opt_argument_str="$opt_argument_str "`genOption "file" "${arg_output_file}"` #
   opt_argument_str="$opt_argument_str "`genOption "type" "${arg_type}"` #
   opt_argument_str="$opt_argument_str "`genOption "fields" "${opt_fields}"`
   opt_argument_str="$opt_argument_str "`genOption "include" "${opt_include}"`
   opt_argument_str="$opt_argument_str "`genOption "errorstop" "${opt_errorstop}"`
   opt_argument_str="$opt_argument_str "`genOption "includebinary" "${opt_includebinary}"`
   opt_argument_str="$opt_argument_str "`genOption "includeregex" "${opt_includeregex}"`
   opt_argument_str="$opt_argument_str "`genOption "filter" "$opt_filter"`
   opt_argument_str="$opt_argument_str "`genOption "sort" "${opt_sort}"`
   opt_argument_str="$opt_argument_str "`genOption "preferedinstance" "${opt_prefinst}"`
   opt_argument_str="$opt_argument_str "`genOption "ssl" "${opt_ssl}"`

   echo $opt_argument_str
}

#
#@description: display debug info
#@argument:
#@return 0 for success
#
function displayDebugInfo
{
   local elem=""

   # user input options
   echo ""
   echo "Debug: User input: "
   echo "$opt_argument_str"
   echo ""
   
   # cs user expect
   echo "Debug: Collection space to export:"
   for elem in ${arr_input_cs[*]}
   do
      echo "${elem}"
   done
   echo ""
 
   # cl user expect
   echo "Debug: Collection to export:"
   for elem in ${arr_input_cl[*]}
   do
      echo "${elem}"
   done
   echo ""
 
   # cs in db
   echo "Debug: Collection space in database:"
   for elem in ${arr_db_cs[*]}
   do
      echo "${elem}"
   done
   echo ""

   # cl in db
   echo "Debug: Collection in database:"
   for elem in ${arr_db_cl[*]}
   do
      echo "${elem}"
   done

   return 0
}

#
#@description: export data
#@argument:
#@return 0 for success
#
function exportData()
{
   local val_path=$1
   local val_type=$2
   local rc=0
   local cs_name=""
   local new_cs_name=""
   local cl_name=""
   local cl_full_name=""
   local new_cl_full_name=""
   local val_item=""
   local val_output_dir=""
   local val_output_file=""
   local val_command=""
   local val_arguments=""
   local val_total_begin_time=""
   local val_total_end_time=""
   local val_begin_time=""
   local val_end_time=""
   local val_total_num=0
   local val_finish_num=0
   local val_error_num=0
   local val_stop_flag=0

   # let dialog leaves in current path
   cd ${CUR_PATH}
   rm -f ${SDBEXPRT_SH_LOG}
   
   val_total_num=${#arr_input_cl[@]}
   val_total_begin_time=`date +"%Y-%m-%d %H:%M:%S"`  

   if [ "2" = "${opt_debug}" ] || [ "3" = "${opt_debug}" ] ; then
      echo ""
      echo "#################################################################################"
      echo ""
   fi
   for val_item in ${arr_input_cl[*]}
   do
      val_stop_flag=0
      cl_full_name=`separateAndRet "$val_item" ":" 0`
      cs_name=`separateAndRet "$cl_full_name" "." 0`
      cl_name=`separateAndRet "$cl_full_name" "." 1`
      new_cl_full_name=`separateAndRet "$val_item" ":" 1`
      new_cs_name=`separateAndRet "$new_cl_full_name" "." 0`
      val_output_dir="$val_path/$new_cs_name"
      val_output_file="$val_output_dir/$new_cl_full_name.$val_type"

      # create output path
      mkdir -p "$val_output_dir"
      
      # get arguments
      val_arguments=`genArgForExport "$cs_name" "$cl_name" "$val_output_file" "$val_type"`
      val_command="${SDBEXPRT} ${val_arguments}" # nerver use "${val_arguments}"

      # display debug info
      if [ "2" = "${opt_debug}" ] || [ "3" = "${opt_debug}" ] ; then
         echo "Begin to export '$cl_full_name' to '$val_output_file'."
      fi
      if [ "3" = "${opt_debug}" ] ; then
         echo "Debug: The command is: "
         echo "${val_command}" # debug
      fi

      # get begin time
      val_begin_time=`date +"%Y-%m-%d %H:%M:%S"`

      # run command to export and check
      if [ "1" = "${opt_debug}" ] ; then
         eval ${val_command} >> ${SDBEXPRT_SH_LOG}
         rc=$?
      else
         eval ${val_command}
         rc=$?
      fi
      if [ 0 -ne $rc ] ; then
         (( val_error_num++ ))
         if [ "true" = "${opt_errorstop}" ] ; then
            val_stop_flag=1
         fi
      else
         (( val_finish_num++ ))
      fi

      # get finish time
      val_end_time=`date +"%Y-%m-%d %H:%M:%S"`
      if [ "2" = "${opt_debug}" ] || [ "3" = "${opt_debug}" ] ; then
         spendTime "$val_begin_time" "$val_end_time"
         echo ""
      fi
 
      # check stop running or not
      if [ 1 -eq ${val_stop_flag} ] ; then
         break
      fi

   done
   if [ "2" = "${opt_debug}" ] || [ "3" = "${opt_debug}" ] ; then
      echo "#################################################################################"
      echo ""
   fi

   val_total_end_time=`date +"%Y-%m-%d %H:%M:%S"`

   # summanrizing the result
   echo "Export result:"
   echo "Total   : ${val_total_num} collection(s)."
   echo "Finish  : ${val_finish_num} collection(s)."
   echo "Fail    : ${val_error_num} collection(s)."
   echo "See ${SDBEXPRT_LOG} or ${SDBEXPRT_SH_LOG} for more detail."
   spendTime "$val_total_begin_time" "$val_total_end_time"

   return $rc
}

################################################################################

# check argument
if [ $# -eq 0 ] ; then
   displayUsage
   exit
fi

# parse arguments
while [ -n "$1" ] && [ $# -gt 0 ]
do
   case "$1" in
   -h | --help)
      displayUsage
      exit ;;
   --version)
      displayVersion $opt_version
      exit ;;
   -s | --hostname)
      opt_hostname=$2
      verifyArgument "--hostname" "$opt_hostname"
      opt_argument_str=${opt_argument_str}" --hostname "${opt_hostname} ;;
   -p | --svcname)
      opt_svcname=$2
      verifyArgument "--svcname" "$opt_svcname"
      opt_argument_str=${opt_argument_str}" --svcname "${opt_svcname} ;;
   -u | --user)
      opt_user=$2
      verifyArgument "--user" "$opt_user"
      opt_argument_str=${opt_argument_str}" --user "${opt_user} ;;
   -w | --password)
      opt_password=$2
      verifyArgument "--password" "$opt_password"
      opt_argument_str=${opt_argument_str}" --password "${opt_password} ;;
   -a | --delchar)
      opt_delchar=$2
      verifyArgument "--delchar" "$opt_delchar"
      opt_argument_str=${opt_argument_str}" --delchar "${opt_delchar} ;;
   -e | --delfield)
      opt_delfield=$2
      verifyArgument "--delfield" "$opt_delfield"
      opt_argument_str=${opt_argument_str}" --delfield "${opt_delfield} ;;
   -r | --delrecord)
      opt_delrecord=$2
      verifyArgument "--delrecord" "$opt_delrecord"
      opt_argument_str=${opt_argument_str}" --delrecord "${opt_delrecord} ;;
   --type)
      opt_type=$2
      verifyArgument "--type" "$opt_type"
      opt_argument_str=${opt_argument_str}" --type "${opt_type} ;;
   --fields)
      opt_fields=$2
      verifyArgument "--fields" "$opt_fields"
      opt_argument_str=${opt_argument_str}" --fields "${opt_fields} ;;
   --include)
      opt_include=$2
      verifyArgument "--include" "$opt_include"
      opt_argument_str=${opt_argument_str}" --include "${opt_include} ;;
   --errorstop)
      opt_errorstop=$2
      verifyArgument "--errorstop" "$opt_errorstop"
      if [ "true" != "${opt_errorstop}" ] && [ "false" != "${opt_errorstop}" ]
      then
         echo "Error: argument of option '--errorstop' should be 'true' or 'false'"
         exit
      fi
      opt_argument_str=${opt_argument_str}" --errorstop "${opt_errorstop} ;;
   --includebinary)
      opt_includebinary=$2
      verifyArgument "--includebinary" "$opt_includebinary"
      opt_argument_str=${opt_argument_str}" --includebinary "${opt_includebinary} ;;
   --includeregex)
      opt_includeregex=$2
      verifyArgument "--includeregex" "$opt_includeregex"
      opt_argument_str=${opt_argument_str}" --includeregex "${opt_includeregex} ;;
   --filter)
      opt_filter=$2
      verifyArgument "--filter" "$opt_filter"
      opt_argument_str=${opt_argument_str}" --filter "${opt_filter} ;;
   --sort)
      opt_sort=$2
      verifyArgument "--sort" "$opt_sort"
      opt_argument_str=${opt_argument_str}" --sort "${opt_sort} ;;
   --preferedinstance)
      opt_prefinst=$2
      verifyArgument "--preferedinstance" "$opt_prefinst"
      opt_argument_str=${opt_argument_str}" --preferedinstance "${opt_prefinst} ;;
   --ssl)
      opt_ssl=$2
      verifyArgument "--ssl" "$opt_ssl"
      opt_argument_str=${opt_argument_str}" --ssl "${opt_ssl} ;;
   -c | --csname)
      opt_csname=$2
      verifyArgument "--csname" "$opt_csname"
      opt_argument_str=${opt_argument_str}" --csname "${opt_csname} ;;
   -l | --clname)
      opt_clname=$2
      verifyArgument "--clname" "$opt_clname"
      opt_argument_str=${opt_argument_str}" --clname "${opt_clname} ;;
   --output)
      opt_output=$2
      verifyArgument "--output" "$opt_output"
      opt_argument_str=${opt_argument_str}" --output "${opt_output} ;; 
   --debug)
      opt_debug=$2
      verifyArgument "--debug" "$opt_debug"
      if [ "1" != "${opt_debug}" ] && [ "2" != "${opt_debug}" ] && [ "3" != "${opt_debug}" ]
      then
         echo "Error: argument of option '--debug' should be 1-3"
         exit
      fi
      opt_argument_str=${opt_argument_str}" --debug "${opt_debug} ;; 
   *)
      echo "Error: unkown option: $1"
      displayUsage
      exit ;;
   esac
   shift # never use "shift 2"
   shift
done
                                            
# check input path
verifyArgumentsForExport

# check input cs and cl name
checkInputCSOrCLNameList "$opt_csname" "cs"
checkInputCSOrCLNameList "$opt_clname" "cl"

# get cs and cl name from command line
getInputCSAndCLNameForExport "$opt_hostname" "$opt_svcname" "$opt_user" "$opt_password" "$opt_csname" "$opt_clname"

# get cs and cl from database
getCSFromDB "$opt_hostname" "$opt_svcname" "$opt_user" "$opt_password"
getCLFromDB "$opt_hostname" "$opt_svcname" "$opt_user" "$opt_password"

# check whether all the input cs and cl exist in database or not
compareCSCLWithDBForExport "$opt_hostname" "$opt_svcname" "$opt_user" "$opt_password" "$opt_csname" "$opt_clname"

# display debug info
if [ "3" = "${opt_debug}" ] ; then
   displayDebugInfo
fi

# begin to import data
exportData "$opt_output" "$opt_type"

