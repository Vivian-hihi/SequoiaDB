#!/bin/bash

CUR_PATH=$(cd `dirname $0`; pwd)

# import common functions
source ${CUR_PATH}/common.sh

SDBIMPRT=`getProgFullPath "sdbimprt"`
PROG_PATH=`dirname ${SDBIMPRT}`
SDB=${PROG_PATH}/sdb

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
opt_insertnum=""
opt_type=""
opt_fields=""
opt_headerline=""
opt_sparse=""
opt_extra=""
opt_linepriority=""
opt_errorstop=""
opt_force=""
opt_ssl=""
opt_input=""
opt_debug=""

opt_argument_str=""

arr_options=( "-h" "-s" "-p" "-u" "-w" "-a" "-e" "-r" "-c" "-l" "-n" "--help" "--version" "--hostname" "--svcname" "--user" "--password" "--delchar" "--delfield" "--delrecord" "--csname" "--clname" "--insertnum" "--file" "--type" "--fields" "--headerline" "--sparse" "--extra" "--linepriority" "--errorstop" "--force" "--ssl" "--input" "--debug" )

################################################################################

#
#@description: display usage
#@argument: null
#@return: null
#
function displayUsage()
{
   # filter '-c [ --csname ]' '-l [ --clname ]' '--file'
   $SDBIMPRT "--help" | sed '/-c /d' | sed '/-l /d' | sed '/--file arg/d'
   echo "  --csname arg           specified the cs list to import, use ',' to "
   echo "                         separate the input cs. If you want to import "
   echo "                         data to another existed cs, use ':' to specify "
   echo "                         the new cs (e.g. --csname 'foo', --csname 'foo:newfoo' "
   echo "                         or --csname 'foo,foo2,foo3:newfoo3') "
   echo "  --clname arg           specified the cl list to import(e.g. --clname 'foo.bar', "
   echo "                         --clname 'foo.bar:newfoo.newbar' "
   echo "                         or --clname 'foo.bar,foo1.bar1,foo2.bar2:newfoo2.newbar2' "
   echo "  --input arg            specified the directory where input files are in "
}

#
#@description: display version
#@argument: null
#@return: null
#
function displayVersion()
{
   $SDBIMPRT $1
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
function verifyArguments()
{
   # check opt_input
   if [ "" = "$opt_input" ] ; then
      echo "Error: option '--input' must be specified."
      exit
   fi
   if [ -d $opt_input ] ; then
      :
   else
      echo "Error: the input path '$opt_input' does not exist or it's not a directory."
      exit
   fi
   if [ -r $opt_input ] ; then
      :
   else
      echo "Error: '$opt_input' is unreadable."
      exit
   fi

   # change relative path to full path
   opt_input=`getFullPath "$opt_input"`

   # check opt_type
   if [ "csv" != "$opt_type" ] && [ "json" != "$opt_type" ] ; then
      echo "Error: option '--type' should be specified as one of the follow: 'csv', 'json'."
      exit
   fi

   return 0
}

#
#@description: gen the arguments for importing collection
#@argument: cs name and cl name
#@return: the argument string, e.g. 
# --hostname susetzb --svcname 11810 --csname foo1 --clname bar1 --file export_result/foo1/foo1.bar1.csv --type csv --fields a,b,c --include true 
#
function genArgForImport()
{
   local val_cs_name=$1
   local val_cl_name=$2
   local val_file=$3
  
   opt_argument_str=""
   opt_argument_str="$opt_argument_str "`genOption "hostname" "${opt_hostname}"`
   opt_argument_str="$opt_argument_str "`genOption "svcname" "${opt_svcname}"`
   opt_argument_str="$opt_argument_str "`genOption "user" "${opt_user}"`
   opt_argument_str="$opt_argument_str "`genOption "password" "${opt_password}"`
   opt_argument_str="$opt_argument_str "`genOption "delchar" "${opt_delchar}"`
   opt_argument_str="$opt_argument_str "`genOption "delfield" "${opt_delfield}"`
   opt_argument_str="$opt_argument_str "`genOption "delrecord" "${opt_delrecord}"`
   opt_argument_str="$opt_argument_str "`genOption "insertnum" "${opt_insertnum}"`
   opt_argument_str="$opt_argument_str "`genOption "file" "${val_file}"` #
   opt_argument_str="$opt_argument_str "`genOption "type" "${opt_type}"`
   opt_argument_str="$opt_argument_str "`genOption "fields" "${opt_fields}"`
   opt_argument_str="$opt_argument_str "`genOption "headerline" "${opt_headerline}"`
   opt_argument_str="$opt_argument_str "`genOption "sparse" "${opt_sparse}"`
   opt_argument_str="$opt_argument_str "`genOption "extra" "${opt_extra}"`
   opt_argument_str="$opt_argument_str "`genOption "linepriority" "${opt_linepriority}"`
   opt_argument_str="$opt_argument_str "`genOption "errorstop" "${opt_errorstop}"`
   opt_argument_str="$opt_argument_str "`genOption "force" "${opt_force}"`
   opt_argument_str="$opt_argument_str "`genOption "ssl" "${opt_ssl}"`
   opt_argument_str="$opt_argument_str "`genOption "csname" "${val_cs_name}"` #
   opt_argument_str="$opt_argument_str "`genOption "clname" "${val_cl_name}"` #

   echo "$opt_argument_str"
}

#
#@description: display debug info
#@argument:
#@return 0 for success
#
function displayDebugInfo
{
   local val_type=""
   local elem=""

   # user input options
   echo ""
   echo "#################################################################################"
   echo ""
   echo "Debug: User input: "
   echo "$opt_argument_str"
   echo ""

   # cs user expect
   echo "Debug: Collection space to import:"
   for elem in ${arr_input_cs[*]}
   do
      echo "${elem}"
   done
   echo ""

   # cl user expect
   echo "Debug: Collection to import:"
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
   echo ""

   # cs in input path
   echo "Debug: Collection space in input path:"
   for elem in ${arr_file_cs[*]}
   do
      echo "${elem}"
   done
   echo ""

   # cl in input path
   echo "Debug: Collection in input path:"
   for elem in ${arr_file_cl[*]}
   do
      echo "${elem}"
   done

   return 0
}

#
#@description: import data
#@arguments: 1. the path where input files are in
#            2. import csv or json files
#@return: 0 for success
#
function importData()
{
   local val_path=$1
   local val_type=$2
   local rc=0
   local cs_name=""
   local cl_name=""
   local cl_full_name=""
   local new_cl_full_name=""
   local val_cs_dir=""
   local val_item=""
   local val_src_file=""
   local val_command=""
   local val_arguments=""
   local val_begin_time=""
   local val_end_time=""

   # let dialog leaves in current path
   cd ${CUR_PATH}

   echo ""
   echo "#################################################################################"
   echo ""
   for val_item in ${arr_input_cl[*]}
   do
      cl_full_name=`separateAndRet "$val_item" ":" 0`
      val_cs_dir=`separateAndRet "$cl_full_name" "." 0`
      new_cl_full_name=`separateAndRet "$val_item" ":" 1`
      cs_name=`separateAndRet "$new_cl_full_name" "." 0`
      cl_name=`separateAndRet "$new_cl_full_name" "." 1`
      val_src_file="$val_path/$val_cs_dir/$cl_full_name.$val_type"
      val_arguments=`genArgForImport "$cs_name" "$cl_name" "$val_src_file"`
      # import

      echo "Begin to import '$val_src_file' to '$new_cl_full_name'."
      val_begin_time=`date +"%Y-%m-%d %H:%M:%S"`
      val_command="${SDBIMPRT} ${val_arguments}" # nerver use "${val_arguments}"
      if [ "true" = "${opt_debug}" ] ; then
         echo "Debug: The command is:"
         echo "${val_command}" # debug
      fi
      
      # run command to import
      eval ${val_command}
      rc=$?
      if [ 0 -ne $rc ] ; then
         echo "Error: error happen."
         echo ""
         break
      fi

      val_end_time=`date +"%Y-%m-%d %H:%M:%S"`
      spendTime "$val_begin_time" "$val_end_time"

      echo ""

   done
   echo "#################################################################################"
   echo ""

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
   -n | --insertnum)
      opt_insertnum=$2
      verifyArgument "--insertnum" "$opt_insertnum"
      opt_argument_str=${opt_argument_str}" --insertnum "${opt_insertnum} ;;
   --type)
      opt_type=$2
      verifyArgument "--type" "$opt_type"
      opt_argument_str=${opt_argument_str}" --type "${opt_type} ;;
   --fields)
      opt_fields=$2
      verifyArgument "--fields" "$opt_fields"
      opt_argument_str=${opt_argument_str}" --fields "${opt_fields} ;;
   --headerline)
      opt_headerline=$2
      verifyArgument "--headerline" "$opt_headerline"
      opt_argument_str=${opt_argument_str}" --headerline "${opt_headerline} ;;
   --sparse)
      opt_sparse=$2
      verifyArgument "--sparse" "$opt_sparse"
      opt_argument_str=${opt_argument_str}" --sparse "${opt_sparse} ;;
   --extra)
      opt_extra=$2
      verifyArgument "--extra" "$opt_extra"
      opt_argument_str=${opt_argument_str}" --extra "${opt_extra} ;;
   --linepriority)
      opt_linepriority=$2
      verifyArgument "--linepriority" "$opt_linepriority"
      opt_argument_str=${opt_argument_str}" --linepriority "${opt_linepriority} ;;
   --errorstop)
      opt_errorstop=$2
      verifyArgument "--errorstop" "$opt_errorstop"
      opt_argument_str=${opt_argument_str}" --errorstop "${opt_errorstop} ;;
   --force)
      opt_force=$2
      verifyArgument "--force" "$opt_force"
      opt_argument_str=${opt_argument_str}" --force "${opt_force} ;;
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
   --input)
      opt_input=$2
      verifyArgument "--input" "$opt_input"
      opt_argument_str=${opt_argument_str}" --input "${opt_input} ;; 
   --debug)
      opt_debug=$2
      verifyArgument "--debug" "$opt_debug"
      if [ "true" != "${opt_debug}" ] && [ "false" != "${opt_debug}" ]
      then
         echo "Error: invalid argument for option '--debug'"
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
verifyArguments

# get cs and cl name from command line
getInputCSAndCLNameForImport "$opt_csname" "$opt_clname" "$opt_input" "$opt_type"

# get cs and cl from local file
getCSAndCLNameFromDir "$opt_input" "$opt_type"

# get cs and cl from database
getCSFromDB "$opt_hostname" "$opt_svcname" "$opt_user" "$opt_password"
getCLFromDB "$opt_hostname" "$opt_svcname" "$opt_user" "$opt_password"

# check whether all the input cs and cl are in local
compareCSCLWithLocalForImport "$opt_csname" "$opt_clname" "$opt_input" "$opt_type"
 
# check whether all the input cs and cl exist in database or not
compareCSCLWithDBForImport "$opt_hostname" "$opt_svcname" "$opt_user" "$opt_password" "$opt_csname" "$opt_clname"

# display debug info
if [ "true" = "${opt_debug}" ] ; then
   displayDebugInfo
fi

# begin to import data
importData "$opt_input" "$opt_type"
if [ 0 != $? ] ; then
   echo "Error: Failed to import all the data to database."
fi
