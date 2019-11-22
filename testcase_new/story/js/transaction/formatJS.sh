#!/bin/bash

# ----------------------------- #
add=false         # +
sub=false         # -
mul=false         # *
div=false         # /
equal=false       # =
_r=true           # \r
_t=true           # \n
# ----------------------------- #

# check line begin with a str
#   ret: "true" | "false"
function is_begin_with_str()
{
  str=$1
  line=$2
  ret_value=false
  test -z "${line}" && echo "${ret_value}" && return
  echo "${line}" | grep "^${str}" >/dev/null
  test $? -eq 0 && ret_value=true && echo "${ret_value}" && return
  echo "${ret_value}"
}

# check line end with a str
#   ret: "true" | "false"
function is_end_with_str()
{
  str=$1
  line=$2
  ret_value=false
  test -z "${line}" && echo "${ret_value}" && return
  echo "${line}" | grep "${str}$" >/dev/null
  test $? -eq 0 && ret_value=true && echo "${ret_value}" && return
  echo "${ret_value}"
}

# check line contain a str
#   ret: "true" | "false"
function is_contain_str()
{
  str=$1
  line=$2
  ret_value=false
  test -z "${line}" && echo "${ret_value}" && return
  echo "${line}" | grep "${str}" >/dev/null
  test $? -eq 0 && ret_value=true && echo "${ret_value}" && return
  echo "${ret_value}"
}

testcase_dir=$1
test -z "${testcase_dir}" && testcase_dir=.
js_files=($(ls ${testcase_dir} | grep ".js$"))
js_file_count=0
for js_file in ${js_files[*]}
do
  js_file=${testcase_dir}/${js_file}
  test "${_t}" = "true" && sed -i -e 's/\t/   /g' ${js_file}
  test "${_r}" = "true" && sed -i -e 's/\r//g' ${js_file}
  test "${add}" = "true" && sed -i -e 's/ *+ */ + /g' ${js_file}
  test "${sub}" = "true" && sed -i -e 's/ *- */ - /g' ${js_file}
  test "${mul}" = "true" && sed -i -e 's/ *\* */ \* /g' ${js_file}
  test "${div}" = "true" && sed -i -e 's/ *\/ */ \/ /g' ${js_file}
  test "${equal}" = "true" && sed -i -e 's/ *= */ = /g' ${js_file}
  sed -i -e 's/ *< */ < /g' ${js_file}
  sed -i -e 's/ *> */ > /g' ${js_file}
  sed -i -e 's/ *= *= */ == /g' ${js_file}
  sed -i -e 's/ *! *= */ != /g' ${js_file}
  sed -i -e 's/ *! *= *= */ !== /g' ${js_file}
  sed -i -e 's/ *< *= */ <= /g' ${js_file}
  sed -i -e 's/ *> *= */ >= /g' ${js_file}
  sed -i -e 's/ *= *= *= */ === /g' ${js_file}
  sed -i -e 's/ *+ *= */ += /g' ${js_file}
  sed -i -e 's/ *- *= */ -= /g' ${js_file}
  sed -i -e 's/ *+ *+ */++/g' ${js_file}
  sed -i -e 's/ *- *- */--/g' ${js_file}
  sed -i -e 's/ *, */, /g' ${js_file}
  sed -i -e 's/ *; */; /g' ${js_file}
  sed -i -e 's/ *) */ )/g' ${js_file}
  sed -i -e 's/ *( */( /g' ${js_file}
  sed -i -e 's/( *)/()/g' ${js_file}

  # handle like "select( xxx )from xxx"
  perl -i -pe 's/\)(?=[A-Za-z]+)/\) /g' ${js_file}
  
  js_file_count=$[${js_file_count}+1]
  echo "file[${js_file_count}]: ${js_file}"

  count=0
  need_space=0
  is_comment_line=false
  touch ./tmp_file
  while read -r line || [ -n "${line}" ]
  do
    count=$[${count}+1]
    skip=false

    # if line is belong comment, then it will skip
    tmp_a=$(is_contain_str "\*\*\*\*\*\*\*\*\*\*\*\*" "${line}")
    if [ "${tmp_a}" = "true" ];then
      if [ "${is_comment_line}" = "false" ];then
        is_comment_line=true
      else
        is_comment_line=false
      fi
    fi

    # if line begin with "/" or is_comment_line="true", then it will skip
    tmp_a=$(is_begin_with_str "/" "${line}")
    if [[ "${is_comment_line}" = "true" || "${tmp_a}" = "true" ]];then
      skip=true
    fi

    # if line begin with "{" and end with "xxx}xxx", then it will skip
    tmp_a=$(is_begin_with_str "{" "${line}" )
    tmp_b=$(is_contain_str "}" "${line}")
    if [[ "${tmp_a}" = "true" && "${tmp_b}" = "true" ]];then
      skip=true
    fi

    # if "}" or "xxx}" exist need_space will decrement 3
    doflag=true
    test "${line}" = "}" && need_space=$[${need_space}-3] && doflag=false
    if [[ "true" = "${doflag}" && "${skip}" = "false" ]];then
      tmp_a=$(is_end_with_str "}" "${line}")
      test "${tmp_a}" = "false" && doflag=false
      if [[ "${doflag}" = "true" && "${skip}" = "false" ]];then
        line=$(echo ${line} | sed 's/}$//g')
      fi
    fi

    # if "{" or "xxx{" or "{xxx" exist need_space will increment 3
    doflag2=true
    addflag2=false
    echoflag2=false
    line2=null
    test "${line}" = "{" && doflag2=false && addflag2=true
    if [[ "true" = "${doflag2}" && "${skip}" = "false" ]];then
      echo "${line}" | grep "{$" >/dev/null
      ret=$?
      if [ ${ret} -ne 0 ];then
        tmp_a=$(is_begin_with_str "{" "${line}")
        test "${tmp_a}" = "false" && doflag2=false
        if [ "${doflag2}" = "true" ];then
          line2=$(echo "${line}" | sed 's/^{//g')
          line="{"
        fi
      else
        line=$(echo "${line}" | sed 's/{$//g')
        echoflag2=true
      fi
    fi

    # add space to the str
    space=""
    for (( i=0; i<${need_space}; i++ ))
    do
      space=${space}" "
    done
    echo "${space}${line}" >>./tmp_file
   
    # echo "xxx}"
    if [[ "${doflag}" = "true" && "${skip}" = "false" ]];then
      tmp_space="${space:0:$[${#space}-3]}"
      echo "${tmp_space}}" >>./tmp_file
    fi
  
    # echo "xxx{"
    if [ "${echoflag2}" = "true" ];then
      tmp_space="${space}"
      echo "${tmp_space}{" >>./tmp_file
    fi

    # echo "{xxx"
    if [ "${line2}" != "null" ];then
      tmp_space="${space}   "
      echo "${tmp_space}${line2}" >>./tmp_file
    fi

    # if skip = "true", then it will not change space num 
    test "${skip}" = "true" && doflag=false && doflag2=false && addflag2=false
    test "${doflag2}" = "true" && need_space=$[${need_space}+3]
    test "${doflag}" = "true" && need_space=$[${need_space}-3]
    test "${addflag2}" = "true" && need_space=$[${need_space}+3]
    # echo ${count}:${need_space}:${skip}
  done < ${js_file}
  cat ./tmp_file > ${js_file}
  rm ./tmp_file
done
