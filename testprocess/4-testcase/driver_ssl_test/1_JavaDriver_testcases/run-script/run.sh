#!/bin/bash
# exec JavaDriver-testcases script
# usage:
#			/opt/tools/java-ssl-test/run-script/run.sh
#	exec all junit testcase-class in the TESTCASELIST_FILE

SCRIPT_DIR=`dirname $0`
LIB_DIR=${SCRIPT_DIR}/../lib
TESTCASE_DIR=${SCRIPT_DIR}/../
PREFIX_STRING="====## "

JUNIT_JAR_FILE=${LIB_DIR}/junit-4.10.jar
SEQUOIADB_JAR_FILE=${LIB_DIR}/sequoiadb.jar
TESTCASELIST_FILE=${SCRIPT_DIR}/testcase-list.txt

echo ${PREFIX_STRING}"script dir: ${SCRIPT_DIR}"
echo ${PREFIX_STRING}"junit: ${JUNIT_JAR_FILE}"
echo ${PREFIX_STRING}"sequoiadb driver: ${SEQUOIADB_JAR_FILE}"

# echo ${PREFIX_STRING}"set CLASSPATH"
# CLASSPATH=${CLASSPATH}:${JUNIT_JAR_FILE}:${SEQUOIADB_JAR_FILE}:${TESTCASE_DIR}
# echo ${PREFIX_STRING}"CLASSPATH=${CLASSPATH}"
LIB_PATH=.:${JUNIT_JAR_FILE}:${SEQUOIADB_JAR_FILE}:${TESTCASE_DIR}

echo

STARTTIME=`date +%Y%m%d%H%M%S`
RESULT_FILE=${STARTTIME}.result

echo ${PREFIX_STRING} beging ${STARTTIME}

for testcase in `cat ${TESTCASELIST_FILE}`
# for testcase in com.sequoiadb.test.Bug_CL_InsertOld
do
    echo 
    echo ${PREFIX_STRING}"begin to  run testcase $testcase"
    echo "java -cp ${LIB_PATH}  org.junit.runner.JUnitCore $testcase"
    java -cp "${LIB_PATH}" org.junit.runner.JUnitCore "$testcase"
    echo ${PREFIX_STRING}"$testcase finish"
    echo
done

echo
exit 0
