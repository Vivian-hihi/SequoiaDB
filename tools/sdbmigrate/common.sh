#!/bin/bash

PROGPATH=$(cd `dirname $0`; pwd)

# define global array
arr_input_cs=()
arr_input_cl=()
arr_file_cs=()
arr_file_cl=()
arr_db_cs=()
arr_db_cl=()
arr_cl_of_dir_cs=()
arr_cl_of_db_cs=()

#define for separate
arr_separate_result=()
arr_separate_result2=()
arr_separate_result3=()

############################ global function ##################################

function getFullPath()
{
   local relative_path=$1
   local cur_path=${PROGPATH}
   local full_path=""
   if [ -d $relative_path ] ; then
      cd $relative_path
      full_path=`pwd`
   else
      echo "Error: '$relative_path' does't exist or it's not a directory."
      exit
   fi
   cd $cur_path

   echo "$full_path"
}

#
#@description: calculate the spend time
#@argument: two times with the format "date +"%Y-%m-%d %H:%M:%S"',
#           e.g. begin_time=`date +"%Y-%m-%d %H:%M:%S"`
#                sleep( 3 )
#                end_time=`date +"%Y-%m-%d %H:%M:%S"`
#                spendTime "$begin_time" "$end_time"
#                Takes: 0h0m3s
#@reurn: the array of the word list, e.g. [ "foo.bar", "foo2.bar2" ]
#
function spendTime()
{
   local arg_begin_time=$1
   local arg_end_time=$2
   local val_begin_time=0
   local val_end_time=0
   local val_time_distance=0
   local val_hour_distance=0
   local val_hour_remainer=0
   local val_min_distance=0
   local val_min_remainer=0

   val_begin_time=$(date +%s -d "$arg_begin_time")
   val_end_time=$(date +%s -d "$arg_end_time")
   val_time_distance=$[ $val_end_time - $val_begin_time ]
   val_hour_distance=$[ $val_time_distance / 3600 ]
   val_hour_remainer=$[ $val_time_distance % 3600 ]
   val_min_distance=$[ $val_hour_remainer / 60 ]
   val_min_remainer=$[ $val_hour_remainer % 60 ]

   echo "Takes: $val_hour_distance""h""$val_min_distance""m""$val_min_remainer""s"
   return 0
}

#
#@description: parse a string to array
#@argument: word list separate by ',', e.g. "foo.bar,foo2.bar2"
#@reurn: the array of the word list, e.g. [ "foo.bar", "foo2.bar2" ]
#
function getArr()
{
   local arg=$1
   local arr=()

   OLD_IFS=$IFS
   IFS=","
   arr=($arg)
   IFS=$OLD_IFS

   echo ${arr[*]}
}

#
#@description: separate by mark
#@argument: 1. input string
#           2. mark
#@return: 0 for ok
#
function separateByMark()
{
   local argument=$1
   local mark=$2
   arr_separate_result=()

   OLD_IFS=$IFS
   IFS="$mark"
   arr_separate_result=($argument)
   IFS=$OLD_IFS

   return 0
}

function separateByMark2()
{
   local argument=$1
   local mark=$2
   arr_separate_result2=()

   OLD_IFS=$IFS
   IFS="$mark"
   arr_separate_result2=($argument)
   IFS=$OLD_IFS

   return 0
}

function separateByMark3()
{
   local argument=$1
   local mark=$2
   arr_separate_result3=()

   OLD_IFS=$IFS
   IFS="$mark"
   arr_separate_result3=($argument)
   IFS=$OLD_IFS

   return 0
}

#
#@description: separate by mark
#@argument: 1. input string
#           2. mark
#           3. return part
#@return: 0 for ok
#
function separateAndRet()
{
   local argument=$1
   local mark=$2
   local index=$3

   separateByMark "$argument" "$mark"
   echo ${arr_separate_result[$index]}
}

function separateAndRet2()
{
   local argument=$1
   local mark=$2
   local index=$3

   separateByMark2 "$argument" "$mark"
   echo "${arr_separate_result2[$index]}"
}

function separateAndRet3()
{
   local argument=$1
   local mark=$2
   local index=$3

   separateByMark3 "$argument" "$mark"
   echo "${arr_separate_result3[$index]}"
}

#
#@description: gen the specified option
#@argument: 1. the type of the option
#           2. the value of the option
#@return: "" or the option, e.g. "--hostname susetzb" 
#
function genOption()
{
   local option=$1
   local value=$2
   local return_str=""

   if [ "" != "$value" ] ; then
      return_str="--${option} ${value}" 
      if [ "filter" = "${option}" ] || [ "sort" = "${option}" ] || [ "preferedinstance" = "${option}" ]
      then
         return_str="--${option} '${value}'"
      fi
      # debug # TODO: check the follow 3 options
      if [ "delchar" = "${option}" ] || [ "delfield" = "${option}" ] || [ "delrecord" = "${option}" ]
      then
         if [ "'" = "${value}" ] ; then
            return_str="--${option} \"${value}\""
         else
            return_str="--${option} '${value}'"
         fi
      fi
   fi

   echo $return_str
}

#
#@description: check whether the name is ok
#@argument: 1. the cs name or cl full name
#           2. specified "cs" or "cl" to check
#@reurn: 0 for success, 1 for failure
#
function verifyName()
{
   local val_name=$1
   local val_type=$2
   local first_char=""
   local cl_name=""
   local val_tmp=""
   local dot_num=0
   local colon_num=0
   local i=0

   if [ "" = "$val_name" ] ; then
       return 1
   fi
 
   if [ "cs" != "$val_type" ] && [ "cl" != "$val_type" ] ; then
       return 1
   fi
    
   first_char=${val_name:0:1}
   if [ "." = "$first_char" ] || [ " " = "$first_char" ] || [ "\$" = "$first_char" ]
   then
      return 1
   fi
 
   # calculate the number of "." and ":"
   for (( i = 0; i < ${#val_name}; ++i ))
   do
      val_tmp=${val_name:i:1}
      if [ "." = "$val_tmp" ] ; then
         (( dot_num++ ))
      fi
      if [ ":" = "$val_tmp" ] ; then
         (( colon_num++ ))
      fi
   done
 
   if [ 0 -ne "$colon_num" ] ; then
      return 1
   fi

   if [ "cs" = "$val_type" ] ; then
     if [ 0 -ne $dot_num ] ; then
        return 1
     fi
   else
      if [ 1 -ne $dot_num ] ; then
         return 1 ;
      fi
   fi
 
   # when checking cs, return 0
   if [ "cs" = "$val_type" ] ; then
      return 0
   fi
 
   # go on checking cl name
   cl_name=`separateAndRet2 "$val_name" "." 1`
   first_char=${cl_name:0:1}
   if [ "" = "$first_char" ] || [ "." = "$first_char" ] || [ " " = "$first_char" ] || [ "\$" = "$first_char" ]
   then
      return 1
   fi
 
   return 0
}

#
#@description: check whether the cs name list or the cl full name list is ok
#@argument: 1. the cs name list or the cl full name list
#           2. specified "cs" or "cl" to check
#@reurn: 0 for success, when invalid name is specified, stop running
#
function checkInputCSOrCLNameList()
{
   local val_name_list=$1
   local val_type=$2
   local arr_tmp=()
   local count=0
   local first_part=""
   local second_part=""
   local name=""
   local i=0

   if [ "cl" = "$val_type" ] ; then
      separateByMark "$val_name_list" ","
   elif [ "cs" = "$val_type" ] ; then
      separateByMark "$val_name_list" ","
   else
      echo "Error: failed to check cs or cl name list for unkown type."
      exit
   fi

   # check
   for (( i = 0; i < ${#arr_separate_result[@]}; i++ ))
   do
      name="${arr_separate_result[$i]}"
      first_part=`separateAndRet3 "$name" ":" 0`
      second_part=`separateAndRet3 "$name" ":" 1`
      if [ "cl" = "$val_type" ] ; then
         verifyName "$first_part" "cl"
         if [ 0 -ne $? ] ; then
            arr_tmp[$count]="$name"
            (( count++ ))
         fi
         if [ "" != "$second_part" ] ; then
            verifyName "$second_part" "cl"
            if [ 0 -ne $? ] ; then
               arr_tmp[$count]="$name"
               (( count++ ))
            fi
         else # get rid of "foo.bar:"
            if [ ":" = "${name: -1}" ] ; then
               arr_tmp[$count]="$name"
            fi
         fi
         
      else
         verifyName "$first_part" "cs"
         if [ 0 -ne $? ] ; then
            arr_tmp[$count]="$name"
            (( count++ ))
         fi
         if [ "" != "$second_part" ] ; then
            verifyName "$second_part" "cs"
            if [ 0 -ne $? ] ; then
               arr_tmp[$count]="$name"
               (( count++ ))
            fi
         else # get rid of "foo:"
            if [ ":" = "${name: -1}" ] ; then
               arr_tmp[$count]="$name"
            fi
         fi
      fi
   done

   if [ 0 -ne ${#arr_tmp[@]} ] ; then
      if [ "cl" = "$val_type" ] ; then
         echo "Error: invalid collection full name: ${arr_tmp[*]}"
      else
         echo "Error: invalid collection space name: ${arr_tmp[*]}"
      fi
      exit
   fi

   return 0
}

#
#@description: get all the collection space from database
#@argument:
#@return: array, the name of the collection space,
#         while error happen, stop running
#
function getCSFromDB()
{
   local hostname=$1
   local svcname=$2
   local user=$3
   local password=$4
   local cs_name_str=""
   local count=0

   if [ "" = "$hostname" ] ; then
      hostname="$DEFAULT_HOSTNAME"
   fi
   if [ "" = "$svcname" ] ; then
      svcname="$DEFAULT_SVCNAME"
   fi
   if [ "" = "$user" ] ; then
      user="$DEFAULT_USER"
   fi
   if [ "" = "$password" ] ; then
      password="$DEFAULT_PASSWORD"
   fi
   
   arr_db_cs=()
   #get all the cs name from database
   # TODO: when error happen in js, 'cs_name_str' is the content of the
   #       error, but, by now, I don't known how to handle it
   cs_name_str=`${SDB} -s "var db = new Sdb( '${hostname}', '${svcname}', '${user}', '${password}' ) ; var cur = db.listCollectionSpaces() ; var record = null ; var returnStr = '' ; while( null != ( record = cur.next() ) ) { var obj = eval( '(' + record + ')' ) ; if ( '' != returnStr ) returnStr = returnStr + ' ' ; returnStr = returnStr + obj['Name'] ; } println(returnStr) ;"`

   # put the names to array
   local count=0
   for name in $cs_name_str
   do
      arr_db_cs[$count]="$name"
      (( count++ ))
   done

   return 0
}

#
#@description: get all the collection from database
#@argument:
#@return: array, the full name of the collection,
#         while error happen, stop running
#
function getCLFromDB()
{
   local hostname=$1
   local svcname=$2
   local user=$3
   local password=$4
   local cl_full_name_str=""
   local count=0

   if [ "" = "$hostname" ] ; then
      hostname="$DEFAULT_HOSTNAME"
   fi
   if [ "" = "$svcname" ] ; then
      svcname="$DEFAULT_SVCNAME"
   fi
   if [ "" = "$user" ] ; then
      user="$DEFAULT_USER"
   fi
   if [ "" = "$password" ] ; then
      password="$DEFAULT_PASSWORD"
   fi
   
   arr_db_cl=()

   #get all the collection full name from database
   # TODO: when error happen in js, 'cl_full_name_str' is the content of the
   #       error, but, by now, I don't known how to handle it
   cl_full_name_str=`${SDB} -s "var db = new Sdb( '${hostname}', '${svcname}', '${user}', '${password}' ) ; var cur = db.listCollections() ; var record = null ; var returnStr = '' ; while( null != ( record = cur.next() ) ) { var obj = eval( '(' + record + ')' ) ; if ( '' != returnStr ) returnStr = returnStr + ' ' ; returnStr = returnStr + obj['Name'] ; } println(returnStr) ;"`

   # put the names to array
   count=0
   for name in $cl_full_name_str
   do
      arr_db_cl[$count]="$name"
      (( count++ ))
   done

   return 0
}

#
#@description: get all the cl under specified cs directory from local
#@argument: 1. the path where input files are in
#           2. cs name
#           3. "json" or "csv"
#@return: 0 for ok
#
function getCLByCSFromDir()
{
   local val_path=$1
   local val_cs=$2
   local val_type=$3
   local count=0
   local err_count=0
   local path=""
   local file=""
   local tmp_arr=()
   local tmp_str=""
   local length=0
   local errno=0

   arr_cl_of_dir_cs=()

   path="$val_path/$val_cs"
   if [ -d $path ] ; then
      cd $path
   else
      echo "Error: failed to get cl under specified cs in local directory, the path '$path' does't exist or it's not a directory."
      exit
   fi

   # TODO: when file's name has space like "foo. bar.csv",
   #       `ls $path` will parse it to 2 files "foo." and "bar.csv"
   for file in `ls $path`
   do
      # check $file is a file or not
      if [ -f $file ] ; then
         :
      else
         echo "Warning: '$file' is not a file in '$path'"
         continue
      fi
      # check $file is named by 'cs.cl.type' or not
      separateByMark "$file" "."

      # check the file's full name
      length=${#arr_separate_result[@]}
      tmp_str="${arr_separate_result[$length - 1]}"
      if [ "json" != "$tmp_str" ] && [ "csv" != "$tmp_str" ] ; then
         echo "Warning: '$file' has unknown format."
         continue
      fi
      
      if [ 3 -ne ${#arr_separate_result[@]} ] || [ "$val_cs" != "${arr_separate_result[0]}" ] || ([ "csv" != "${arr_separate_result[2]}" ] && [ "json" != "${arr_separate_result[2]}" ]) || [ "" = "${arr_separate_result[1]}" ]
      then
         tmp_arr[$err_count]="$file"
         (( err_count++ ))
         continue
      fi

      # check the cl name
      # just check the cl part, not the cs part
      verifyName "${arr_separate_result[1]}" "cs"
      errno=$?
      if [ 0 -ne $errno ] ; then
         tmp_arr[$err_count]="$file"
         (( err_count++ ))
         continue
      fi

      # record the cl
      if [ "$val_type" = "${arr_separate_result[2]}" ] ; then
         separateByMark "$file" "."
         arr_cl_of_dir_cs[$count]="${arr_separate_result[0]}.${arr_separate_result[1]}"
         (( count++ ))
      fi
   done

   if [ 0 -ne ${#tmp_arr[@]} ] ; then
      echo "Error: the follow files' name are invalid in '$path': ${tmp_arr[@]}"
      exit
   fi

   return 0
}

#
#@description: get all the cl under specified cs directory from local
#@argument: 1. the path where input files are in
#@return: 0 for ok
#
function getCSFromDir()
{
   local val_path=$1
   local count=0

   arr_file_cs=()

   for file in `ls $val_path`
   do
      if [ -d "$val_path/$file" ] ; then
         arr_file_cs[$count]="$file"
         (( count++ ))
      fi
   done

   return 0
}

#
#@description: get cl full name from local directory
#@argument: 1. the path where input files are in
#           2. "json" or "csv"
#@return: 0 for ok
#
function getCLFromDir()
{
   local val_path=$1
   local val_type=$2
   local count=0
   local path=""
   local csname=""

   arr_file_cl=()

   for csname in ${arr_file_cs[*]}
   do
      path="$val_path/$csname"
      if [ -d $path ] ; then
         cd $path
      else
         echo "Error: failed to get cl from local directory, the path '$path' does't exist or it's not a directory."
         exit
      fi

      for file in `ls $path`
      do
         # $file is a file or not
         if [ -f $file ] ; then
            :
         else
            continue
         fi
         # $file is named by 'cs.cl.type' or not
         separateByMark "$file" "."
         if [ 3 -ne ${#arr_separate_result[@]} ] || [ "$csname" != "${arr_separate_result[0]}" ] || [ "$val_type" != "${arr_separate_result[2]}" ] || [ "" = "${arr_separate_result[1]}" ]
         then
            continue
         fi
         # record the collection
         separateByMark "$file" "."
         arr_file_cl[$count]="${arr_separate_result[0]}.${arr_separate_result[1]}"
         (( count++ ))
      done
   done

   return 0
}

#
#@description: get cs and cl full name from local directory
#@argument: 1. the path where input files are in
#           2. "json" or "csv"
#@return: 0 for ok
#
function getCSAndCLNameFromDir()
{
   local val_path=$1
   local val_type=$2

   # check the path
   if [ -d $val_path ] ; then
      cd $val_path
   else
      echo "Error: the path '$val_path' specified by option '--input' does't exist or it's not a directory."
      exit
   fi

   # get cs from directory
   getCSFromDir "$val_path"

   #TODO: is it the right place to check
   if [ 0 -eq ${#arr_file_cs[@]} ] ; then
      echo "Error: no collection space in specified path '$val_path'"
      exit
   fi
   
   # get cl from directory
   getCLFromDir "$val_path" "$val_type"

   return 0
}

#
#@description: get all the cl under specified cs from database
#@argument: 1. hostname 2. svcname
#           3. user 4. password
#           5. cs name
#@return: 0 for ok
#
function getCLByCSFromDB()
{
   local arg_hostname=$1
   local arg_svcname=$2
   local arg_user=$3
   local arg_password=$4
   local arg_cs_name=$5
   local cs_name=""
   local cl_full_name=""
   local count=0
   local i=0

   arr_cl_of_db_cs=()

   getCLFromDB "$arg_hostname" "$arg_svcname" "$arg_user" "$arg_password"
   for cl_full_name in ${arr_db_cl[@]}
   do
      cs_name=`separateAndRet "$cl_full_name" "." "0"`
      if [ "$arg_cs_name" = "$cs_name" ] ; then
         arr_cl_of_db_cs[$count]="$cl_full_name"
         (( count++ ))
      fi
   done

   return 0
}

############################ function for import ##############################

#
#@description: get input cs and cl full name
#@argument: 1. cs name list 2.cl name list 
#           3. the path where the input files are in
#           4. input json or csv
#@return: 0 for ok, when error happen, stop running
#
function getInputCSAndCLNameForImport()
{
   local cs_name_list=$1
   local cl_name_list=$2
   local val_path=$3
   local val_type=$4
   local arr_for_cs=()
   local arr_for_cl=()
   local tmp_cs_name=""
   local tmp_cl_name=""
   local tmp_cs_new_name=""
   local tmp_cl_new_name=""
   local tmp_str=""
   local cs_count=0
   local cl_count=0
   local flag=0
   local i=0
   local j=0

   # init global array
   arr_input_cs=()
   arr_input_cl=()

   # when not specify cs and cl list
   if [ "" = "$cs_name_list" ] && [ "" = "$cl_name_list" ] ; then
      # get cs from dir
      getCSFromDir "$val_path"
      for tmp_cs_name in ${arr_file_cs[*]}
      do
         arr_input_cs[$cs_count]="$tmp_cs_name:$tmp_cs_name"
         (( cs_count++ ))
      done

      # get cl from dir
      getCLFromDir "$val_path" "$val_type"
      for tmp_cl_name in ${arr_file_cl[*]}
      do
         arr_input_cl[$cl_count]="$tmp_cl_name:$tmp_cl_name"
         (( cl_count++ ))
      done

      return 0
   fi

   # check input cs and cl list
   checkInputCSOrCLNameList "$cs_name_list" "cs"
   checkInputCSOrCLNameList "$cl_name_list" "cl"

   # get cs names from cs name list
   arr_for_cs=(`getArr "$cs_name_list"`)

   for (( i = 0; i < ${#arr_for_cs[@]}; i++ ))
   do
      tmp_cs_name=`separateAndRet "${arr_for_cs[$i]}" ":" 0`
      # we are going to input cs:newcs to arr_input_cs
      if [ "$tmp_cs_name" = "${arr_for_cs[$i]}" ] ; then
         tmp_cs_name="$tmp_cs_name:$tmp_cs_name"
      else
         tmp_cs_name="${arr_for_cs[$i]}"
      fi
      flag=0
      for elem in ${arr_input_cs[*]}
      do
         if [ "$elem" != "$tmp_cs_name" ] ; then
            continue
         else
            flag=1
            break
         fi
      done
      if [ 0 -eq $flag ] ; then
         arr_input_cs[$cs_count]="$tmp_cs_name"
         (( cs_count++ ))
      fi
   done

   # get cl full name from cs name list
   for (( i = 0; i < ${#arr_input_cs[@]} ; i++ ))
   do
      tmp_cl_new_name=""
      tmp_cs_name=`separateAndRet "${arr_input_cs[$i]}" ":" 0`
      tmp_cs_new_name=`separateAndRet "${arr_input_cs[$i]}" ":" 1`
      # get cl about the specified cs from dir
      getCLByCSFromDir "$val_path" "$tmp_cs_name" "$val_type"
      for (( j = 0; j < ${#arr_cl_of_dir_cs[@]}; j++ ))
      do
         tmp_cl_new_name="${arr_cl_of_dir_cs[$j]}"
         tmp_str=`separateAndRet "$tmp_cl_new_name" "." 1`
         tmp_cl_new_name="$tmp_cl_new_name:$tmp_cs_new_name.$tmp_str"
         arr_input_cl[$cl_count]="$tmp_cl_new_name"
         (( cl_count++ ))
      done
   done

   # get cs names from cl name list
   arr_for_cl=(`getArr "$cl_name_list"`)

   for (( i = 0; i < ${#arr_for_cl[@]}; i++ ))
   do
      tmp_cl_name=`separateAndRet "${arr_for_cl[$i]}" ":" 0`
      tmp_cs_name=`separateAndRet "$tmp_cl_name" "." 0`
      tmp_cl_new_name=`separateAndRet "${arr_for_cl[$i]}" ":" 1`
      if [ "" = "$tmp_cl_new_name" ] ; then
         tmp_cs_new_name="$tmp_cs_name"
      else
         tmp_cs_new_name=`separateAndRet "$tmp_cl_new_name" "." 0`
      fi
      tmp_cs_name="$tmp_cs_name:$tmp_cs_new_name"
      flag=0
      for elem in ${arr_input_cs[*]}
      do
         if [ "$elem" != "$tmp_cs_name" ] ; then
            continue
         else
            flag=1
            break
         fi
      done
      if [ 0 -eq $flag ] ; then
         arr_input_cs[$cs_count]="$tmp_cs_name"
         (( cs_count++ ))
      fi
   done

   # get cl full name from cl name list
   for (( i = 0; i < ${#arr_for_cl[@]}; i++ ))
   do
      tmp_cl_name=`separateAndRet "${arr_for_cl[$i]}" ":" 0`
      if [ "$tmp_cl_name" = "${arr_for_cl[$i]}" ] ; then
         tmp_cl_name="$tmp_cl_name:$tmp_cl_name"
      else
         tmp_cl_name="${arr_for_cl[$i]}"
      fi
      flag=0
      for elem in ${arr_input_cl[*]}
      do
         if [ "$elem" != "$tmp_cl_name" ] ; then
            continue
         else
            flag=1
            break
         fi
      done
      if [ 0 -eq $flag ] ; then
         arr_input_cl[$cl_count]="$tmp_cl_name"
         (( cl_count++ ))
      fi
   done


   return 0
}

#
#@description: check whether the input cs and cl exist in local directory
#@argument: 1. cs name list 2.cl name list 
#           3. the path where the input files are in
#           4. input json or csv
#@return: 0 for ok, when error happen, stop running
#
function compareCSCLWithLocalForImport()
{
   local cs_name_list=$1
   local cl_name_list=$2
   local val_path=$3
   local val_type=$4
   local cs_name=""
   local cl_name=""
   local input_cs=""
   local input_cl=""
   local local_cs=""
   local local_cl=""
   local arr_tmp=()
   local count=0
   local flag=0

   # check cs with local
   for input_cs in ${arr_input_cs[*]}
   do
      cs_name=`separateAndRet "$input_cs" ":" 0`
      flag=0
      for local_cs in ${arr_file_cs[*]}
      do
         if [ "$cs_name" = "$local_cs" ] ; then
            flag=1
            break
         fi
      done
      if [ 0 -eq $flag ] ; then
         arr_tmp[$count]="$cs_name"
         (( count++ ))
      fi
   done

   if [ 0 -ne ${#arr_tmp[@]} ] ; then
      echo "Error: the follow collection spaces do not exist in $val_path: ${arr_tmp[*]}"
      exit ;
   fi
   
   # check cl with local
   count=0
   arr_tmp=()
   for input_cl in ${arr_input_cl[*]}
   do
      cl_name=`separateAndRet "$input_cl" ":" 0`
      flag=0
      for local_cl in ${arr_file_cl[*]}
      do
         if [ "$cl_name" = "$local_cl" ] ; then
            flag=1
            break
         fi
      done
      if [ 0 -eq $flag ] ; then
         arr_tmp[$count]="$cl_name"
         (( count++ ))
      fi
   done

   if [ 0 -ne ${#arr_tmp[@]} ] ; then
      echo "Error: the follow collections do not exist in sub directory of '$val_path' with the type of '$val_type': ${arr_tmp[*]}"
      exit ;
   fi

   return 0
}

#
#@description: check whether the input cs and cl exist in database
#@argument: 1. hostname 2. svcname 3.user name 4. password
#           5. cs name list 6.cl name list
#@return: 0 for ok, when error happen, stop running
#
function compareCSCLWithDBForImport()
{
   local val_hostname=$1
   local val_svcname=$2
   local val_user=$2
   local val_password=$4
   local cs_name_list=$5
   local cl_name_list=$6
   local cs_name=""
   local cl_name=""
   local input_cs=""
   local input_cl=""
   local db_cs=""
   local db_cl=""
   local arr_tmp=()
   local count=0
   local flag=0

   # check cs with db
   for input_cs in ${arr_input_cs[*]}
   do
      cs_name=`separateAndRet "$input_cs" ":" 1`
      flag=0
      for db_cs in ${arr_db_cs[*]}
      do
         if [ "$cs_name" = "$db_cs" ] ; then
            flag=1
            break
         fi
      done
      if [ 0 -eq $flag ] ; then
         arr_tmp[$count]="$cs_name"
         (( count++ ))
      fi
   done

   if [ 0 -ne ${#arr_tmp[@]} ] ; then
      echo "Error: the follow collection spaces do not exist in database: ${arr_tmp[*]}"
      exit ;
   fi
   
   # check cl with local
   count=0
   arr_tmp=()
   for input_cl in ${arr_input_cl[*]}
   do
      cl_name=`separateAndRet "$input_cl" ":" 1`
      flag=0
      for db_cl in ${arr_db_cl[*]}
      do
         if [ "$cl_name" = "$db_cl" ] ; then
            flag=1
            break
         fi
      done
      if [ 0 -eq $flag ] ; then
         arr_tmp[$count]="$cl_name"
         (( count++ ))
      fi
   done

   if [ 0 -ne ${#arr_tmp[@]} ] ; then
      echo "Error: the follow collections do not exist in database': ${arr_tmp[*]}"
      exit ;
   fi

   return 0
}

############################ function for import ##############################

#
#@description: get input cs and cl full name
#@argument: 1. hostname 2. svcname 3.user name 4. password
#           5. cs name list 6.cl name list
#@return: 0 for ok, when error happen, stop running
#
function getInputCSAndCLNameForExport()
{
   local arg_hostname=$1
   local arg_svcname=$2
   local arg_user=$3
   local arg_password=$4
   local cs_name_list=$5
   local cl_name_list=$6
   local arr_for_cs=()
   local arr_for_cl=()
   local tmp_cs_name=""
   local tmp_cl_name=""
   local tmp_cs_new_name=""
   local tmp_cl_new_name=""
   local tmp_str=""
   local cs_count=0
   local cl_count=0
   local elem=""
   local flag=0
   local i=0
   local j=0

   # init global array
   arr_input_cs=()
   arr_input_cl=()

   # when not specify cs and cl list
   if [ "" = "$cs_name_list" ] && [ "" = "$cl_name_list" ] ; then
      # get cs from db
      getCSFromDB "$arg_hostname" "$arg_svcname" "$arg_user" "$arg_password"
      for tmp_cs_name in ${arr_db_cs[*]}
      do
         arr_input_cs[$cs_count]="$tmp_cs_name:$tmp_cs_name"
         (( cs_count++ ))
      done

      # get cl from db
      getCLFromDB "$arg_hostname" "$arg_svcname" "$arg_user" "$arg_password"
      for tmp_cl_name in ${arr_db_cl[*]}
      do
         arr_input_cl[$cl_count]="$tmp_cl_name:$tmp_cl_name"
         (( cl_count++ ))
      done

      return 0
   fi

   # check input cs and cl list
   checkInputCSOrCLNameList "$cs_name_list" "cs"
   checkInputCSOrCLNameList "$cl_name_list" "cl"

   # get cs names from cs name list 
   arr_for_cs=(`getArr "$cs_name_list"`)
  
   for (( i = 0; i < ${#arr_for_cs[@]}; i++ ))
   do
      tmp_cs_name=`separateAndRet "${arr_for_cs[$i]}" ":" 0`
      # we are going to input cs:newcs to arr_input_cs
      if [ "$tmp_cs_name" = "${arr_for_cs[$i]}" ] ; then
         tmp_cs_name="$tmp_cs_name:$tmp_cs_name"
      else
         tmp_cs_name="${arr_for_cs[$i]}"
      fi
      flag=0
      for elem in ${arr_input_cs[*]}
      do
         if [ "$elem" != "$tmp_cs_name" ] ; then
            continue
         else
            flag=1
            break
         fi
      done
      if [ 0 -eq $flag ] ; then
         arr_input_cs[$cs_count]="$tmp_cs_name"
         (( cs_count++ ))
      fi
   done

   # get cl full name from cs name list
   for (( i = 0; i < ${#arr_input_cs[@]} ; i++ ))
   do
      tmp_cl_new_name=""
      tmp_cs_name=`separateAndRet "${arr_input_cs[$i]}" ":" 0`
      tmp_cs_new_name=`separateAndRet "${arr_input_cs[$i]}" ":" 1`
      # get cl under the specified cs from database
      getCLByCSFromDB "$arg_hostname" "$arg_svcname" "$arg_user" "$arg_password" "$tmp_cs_name"
      for (( j = 0; j < ${#arr_cl_of_db_cs[@]}; j++ ))
      do
         tmp_cl_new_name="${arr_cl_of_db_cs[$j]}"
         tmp_str=`separateAndRet "$tmp_cl_new_name" "." 1`
         tmp_cl_new_name="$tmp_cl_new_name:$tmp_cs_new_name.$tmp_str"
         arr_input_cl[$cl_count]="$tmp_cl_new_name"
         (( cl_count++ ))
      done
   done

   # get cs names from cl name list
   arr_for_cl=(`getArr "$cl_name_list"`)

   for (( i = 0; i < ${#arr_for_cl[@]}; i++ ))
   do
      tmp_cl_name=`separateAndRet "${arr_for_cl[$i]}" ":" 0`
      tmp_cs_name=`separateAndRet "$tmp_cl_name" "." 0`
      tmp_cl_new_name=`separateAndRet "${arr_for_cl[$i]}" ":" 1`
      if [ "" = "$tmp_cl_new_name" ] ; then
         tmp_cs_new_name="$tmp_cs_name"
      else
         tmp_cs_new_name=`separateAndRet "$tmp_cl_new_name" "." 0`
      fi
      tmp_cs_name="$tmp_cs_name:$tmp_cs_new_name"
      flag=0
      for elem in ${arr_input_cs[*]}
      do
         if [ "$elem" != "$tmp_cs_name" ] ; then
            continue
         else
            flag=1
            break
         fi
      done
      if [ 0 -eq $flag ] ; then
         arr_input_cs[$cs_count]="$tmp_cs_name"
         (( cs_count++ ))
      fi
   done

   # get cl full name from cl name list
   for (( i = 0; i < ${#arr_for_cl[@]}; i++ ))
   do
      tmp_cl_name=`separateAndRet "${arr_for_cl[$i]}" ":" 0`
      if [ "$tmp_cl_name" = "${arr_for_cl[$i]}" ] ; then
         tmp_cl_name="$tmp_cl_name:$tmp_cl_name"
      else
         tmp_cl_name="${arr_for_cl[$i]}"
      fi
      flag=0
      for elem in ${arr_input_cl[*]}
      do
         if [ "$elem" != "$tmp_cl_name" ] ; then
            continue
         else
            flag=1
            break
         fi
      done
      if [ 0 -eq $flag ] ; then
         arr_input_cl[$cl_count]="$tmp_cl_name"
         (( cl_count++ ))
      fi
   done 
   
   return 0
}

#
#@description: check whether the input cs and cl exist in database
#@argument: 1. hostname 2. svcname 3.user name 4. password
#           5. cs name list 6.cl name list
#@return: 0 for ok, when error happen, stop running
#
function compareCSCLWithDBForExport()
{
   local val_hostname=$1
   local val_svcname=$2
   local val_user=$3
   local val_password=$4
   local cs_name_list=$5
   local cl_name_list=$6
   local val_path=$7
   local val_type=$8
   local cs_name=""
   local cl_name=""
   local input_cs=""
   local input_cl=""
   local db_cs=""
   local db_cl=""
   local arr_tmp=()
   local count=0
   local flag=0
 
   # check cs with db
   for input_cs in ${arr_input_cs[*]}
   do
      cs_name=`separateAndRet "$input_cs" ":" 0`
      flag=0
      for db_cs in ${arr_db_cs[*]}
      do
         if [ "$cs_name" = "$db_cs" ] ; then
            flag=1
            break
         fi
      done
      if [ 0 -eq $flag ] ; then
         arr_tmp[$count]="$cs_name"
         (( count++ ))
      fi
   done

   if [ 0 -ne ${#arr_tmp[@]} ] ; then
      echo "Error: the follow collection spaces do not exist in database: ${arr_tmp[*]}"
      exit ;
   fi

   # check cl with db
   count=0
   arr_tmp=()
   for input_cl in ${arr_input_cl[*]}
   do
      cl_name=`separateAndRet "$input_cl" ":" 0`
      flag=0
      for db_cl in ${arr_db_cl[*]}
      do
         if [ "$cl_name" = "$db_cl" ] ; then
            flag=1
            break
         fi
      done
      if [ 0 -eq $flag ] ; then
         arr_tmp[$count]="$cl_name"
         (( count++ ))
      fi
   done

   if [ 0 -ne ${#arr_tmp[@]} ] ; then
      echo "Error: the follow collections do not exist in database': ${arr_tmp[*]}"
      exit ;
   fi

   return 0
}
