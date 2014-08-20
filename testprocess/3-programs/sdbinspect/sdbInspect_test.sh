#!/usr/bin/sh

source config.ini

function checkrc_succ()
{
   if [ $1 -ne 0 ];then
      echo "sdbInspect execute error,line = $2"
      exit
   fi
}

function checkrc_fail()
{
   if [ $1 -eq 0 ];then
      echo "sdbInspect execute error,line = $2"
      exit
   fi
}

function checkReport()
{
   if [ ! -f $1 ];then
      echo "the report file is not exist,line = $2"
      exit
   fi

   count=$(grep '$oid' $1 | wc -l)

   if [ $count -ne $2 ];then
      echo "the record is different,line = $3"
      exit
   fi
}

function clean()
{
   if [ -e $1 ];then
      rm $1
   fi

   if [ -e $2 ];then
      rm $2
   fi
}

function testcase1()
{
   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 101; i++ ){db.$cs_name.$cl_name.insert({'id':i})}"
   $sdb "quit"

   $proc -d $coord -c $cs_name -l $cl_name -t 1 -o $report_name -w 'collection'
   checkrc_succ $? $LINENO
   count=101
   checkReport $report_file $count $LINENO

   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 100; i++ ){db.$cs_name.$cl_name.remove({'id':i})}"
   $sdb "quit"

   $proc -d $coord -f $report_name -t 5 -o $report_name -w 'collection'
   checkrc_succ $? $LINENO
   count=1
   checkReport $report_file $count $LINENO

   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "db.$cs_name.$cl_name.remove({'id':100})"
   $sdb "quit"

   #clean $report_name $report_file

}

function testcase2()
{
   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 101; i++ ){db.$cs_name.$cl_name.insert({'id':i})}"
   $sdb "quit"

   $proc -d $coord -c $cs_name -l $cl_name -t 1 -o $report_name -w 'collection'
   checkrc_succ $? $LINENO
   count=101
   checkReport $report_file $count $LINENO

   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10; i++ ){db.$cs_name.$cl_name.remove({'id':i})}"
   $sdb "quit"

   $proc -d $coord -f $report_name -t 1 -o $report_name -w 'collection'
   checkrc_succ $? $LINENO
   count=91
   checkReport $report_file $count $LINENO

   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 10; i < 101; i++ ){db.$cs_name.$cl_name.remove({'id':i})}"
   $sdb "quit"

   #clean $report_name $report_file


}

function testcase3()
{
   if [ ! -e 'new_file' ];then
      touch 'new_file'
      echo 'hello world' >> 'new_file'
   fi
   $proc -d $coordAddr -f 'new_file' -t 1 -w 'collection' -o 'temp'
   checkrc_fail $? $LINENO
}

function testcase4()
{
   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10; i++ ){db.$cs_name.$cl_name.insert({'id':i})}"
   $sdb "quit"

   $proc -d $coord -g $group_name -c $cs_name -l $cl_name -t 1 -o $report_name -w 'group'
   checkrc_succ $? $LINENO
   count=10
   checkReport $report_file $count $LINENO

   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10; i++ ){db.$cs_name.$cl_name.remove({'id':i})}"
   $sdb "quit"
   #clean $report_name $report_file

}

function testcase5()
{
   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl_name.insert({'id':i});db.$cs_name.$cl1.insert({'id':i})}"
   $sdb "quit"

   $proc -d $coord -c $cs_name -t 1 -o $report_name -w 'collection'
   checkrc_succ $? $LINENO
   count=20
   checkReport $report_file $count $LINENO

   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl_name.remove({'id':i});db.$cs_name.$cl1.remove({'id':i})}"
   $sdb "quit"

   #clean $report_name $report_file

}


function testcase6()
{
   $proc -d $coord -g $group_name -l $cl_name -t 1 -o $report_name -w 'group'
   checkrc_fail $? $LINENO
}

function testcase7()
{
   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl_name.insert({'id':i})}"
   $sdb "quit"

   $proc -d $coord -c $cs_name -t 1 -l $cl_name -w 'collection' -o $report_name
   checkrc_succ $? $LINENO
   count=10
   checkReport $report_file $count $LINENO

   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl_name.remove({'id':i})}"
   $sdb "quit"


   #clean $report_name $report_file

}


function testcase8()
{
   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl_name.insert({'id':i});db.$cs_name.$cl1.insert({'id':i})}"
   $sdb "quit"

   $proc -d $coord -g $group_name -t 1 -o $report_name -w 'collection'
   checkrc_succ $? $LINENO
   count=20
   checkReport $report_file $count $LINENO

   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl_name.remove({'id':i});db.$cs_name.$cl1.remove({'id':i})}"
   $sdb "quit"

   #clean $report_name $report_file

}

function testcase9()
{
   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl_name.insert({'id':i});db.$cs_name.$cl1.insert({'id':i})}"
   $sdb "quit"

   $proc -d $coord -c $cs_name -t 1 -o $report_name -w 'collection'
   checkrc_succ $? $LINENO
   count=20
   checkReport $report_file $count $LINENO

   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl_name.remove({'id':i});db.$cs_name.$cl1.remove({'id':i})}"
   $sdb "quit"

   #clean $report_name $report_file

}



function testcase10()
{
   $proc -d $coordAddr -l 'temp_cl' -t 1 -w 'collection'
   checkrc_fail $? $LINENO
}


function testcase11()
{
   #temp_group is not exist
   $proc -d $coord -g 'temp_group' -t 1 -w 'collection'
   checkrc_fail $? $LINENO

   $proc -d $coord -g 'temp_group' -c $cs_name -t 1 -w 'group' -o 'temp'
   checkrc_fail $? $LINENO

   $proc -d $coord -g 'temp_group' -l $cl_name -t 1 -w 'collection' -o 'temp'
   checkrc_fail $? $LINENO

   $proc -d $coord -g 'temp_group' -c $cs_name -l $cl_name -t 1 -w 'group' -o 'temp'
}

function testcase12()
{
   #temp_cs is not exist
   $proc -d $coord -c 'temp_cs' -t 1 -w 'collection' -o 'temp'
   checkrc_fail $? $LINENO

   $proc -d $coord -c $cs_name  -t 1 -l 'temp_cl' -w 'collection' -o 'temp'
   checkrc_fail $? $LINENO
}

:<<!
function testcase13()
{
   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl_10.insert({'id':i})}"
   $sdb "quit"

   $proc -d $coord -g $cs_name -t 1 -o $report_name -w 'collection'
   checkrc_succ $? $LINENO
   count=10
   checkReport $report_file $count $LINENO

   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl_10.remove({'id':i})}"
   $sdb "quit"

   #clean $report_name $report_file


}

function testcase14()
{
   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl_100.insert({'id':i})}"
   $sdb "quit"

   $proc -d $coord -g $cs_name -t 1 -o $report_name -w 'collection'
   checkrc_succ $? $LINENO
   count=10
   checkReport $report_file $count $LINENO

   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl_100.remove({'id':i})}"
   $sdb "quit"

   #clean $report_name $report_file

}

function testcase15()
{
   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl_1000.insert({'id':i})}"
   $sdb "quit"

   $proc -d $coord -g $cs_name -t 1 -o $report_name -w 'collection'
   checkrc_succ $? $LINENO
   count=10
   checkReport $report_file $count $LINENO

   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl_1000.remove({'id':i})}"
   $sdb "quit"

   #clean $report_name $report_file

}
!

function testcase16()
{
   $proc -b
   checkrc_succ $? $LINENO

   $proc -d -g
   checkrc_fail $? $LINENO

   $proc -a
   checkrc_fail $? $LINENO
}


function testcase17()
{
   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl1.insert({'id':i})}"
   $sdb "quit"

   $proc -d $coord -c $cs_name -t 1 -l $cl_main -w 'collection' -o $report_name
   checkrc_succ $? $LINENO
   count=10
   checkReport $report_file $count $LINENO

   $sdb "var db = new Sdb('$dataAddr',$dataPort)"
   $sdb "for(var i = 0; i < 10;i++){db.$cs_name.$cl1.remove({'id':i})}"
   $sdb "quit"


   #clean $report_name $report_file

}

function runtest()
{
   testcase1
   testcase2
   testcase3
   testcase4
   testcase5
   testcase6
   testcase7
   testcase8
   testcase9
   testcase10
   testcase11
   testcase12
#   testcase13
#   testcase14
#   testcase15
   testcase16
   testcase17
}

#testcase17
#run all testcase
runtest

