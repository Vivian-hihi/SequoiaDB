/***********************************************************************
* @Description : test export with -a random ascii
*                seqDB-15652:自定义任意16进制、10进制ascii码为字符串分隔符，导出到json文件
*								 seqDB-15654:自定义任意16进制、10进制ascii码为字符串分隔符，导出到csv文件         
* @author      : wangkexin
* 
************************************************************************/
var csname = COMMCSNAME ;
var clname = COMMCLNAME + "_sdbexprt15652" ;
var clname1 = COMMCLNAME + "_sdbimprt15652" ;
var doc = { a: 1 , b: "exprtTest" } ;
var expRecs = [ "{\"a\":1,\"b\":\"exprtTest\"}" ] ;
var cl = commCreateCL( db, csname, clname, 0 ) ;
var cl1 = commCreateCL( db, csname, clname1, 0 ) ;
   
main() ;

function main()
{
   cl.insert( doc ) ; 
  
   testExprtImprtJson("abc0x2A0x3f","abc*?") ;
   check()
   testExprtImprtJson("0x2Aabc0x3f","'*abc?'") ;
   check()
   testExprtImprtJson("\\\\107a","'ka'") ;
   check()
   testExprtImprtJson("\\\\1071","'k1'") ;
   check()
   testExprtImprtJson("0x2A","'*'") ;
   check()
   testExprtImprtJson("0xx","'0xx'") ;
   check()
   testExprtImprtJson("分隔符","'分隔符'") ;
   check()
   
   testExprtImprtCsv("abc0x2A0x3f","abc*?") ;
   check()
   testExprtImprtCsv("0x2Aabc0x3f","'*abc?'") ;
   check()
   testExprtImprtCsv("\\\\107a","'ka'") ;
   check()
   testExprtImprtCsv("\\\\1071","'k1'") ;
   check()
   testExprtImprtCsv("0x2A","'*'") ;
   check()
   testExprtImprtCsv("0xx","'0xx'") ;
   check()
   testExprtImprtCsv("分隔符","'分隔符'") ;
   check()
   
   
   commDropCL( db, csname, clname ) ;
   commDropCL( db, csname, clname1 ) ;
}

function testExprtImprtJson(asc,asc1)
{
   var jsonfile = workDir + "sdbexprt15652.json" ;
   cmd.run( "rm -rf " + jsonfile ) ;
   println( "json ascii for delchar is: " + asc ) ;
   var command = installPath + "bin/sdbexprt" +
                 " -s " + COORDHOSTNAME +
                 " -p " + COORDSVCNAME + 
                 " -c " + csname + 
                 " -l " + clname +
                 " --file " + jsonfile + 
                 " --type json" +
                 " -a " + asc + 
                 " --fields a,b" ;
  testRunCommand( command ) ;
   
  command = installPath + "bin/sdbimprt" +
             " -s " + COORDHOSTNAME +
             " -p " + COORDSVCNAME +
             " -c " + csname +
             " -l " + clname1 +
             " --file " + jsonfile +
             " --type json" +
             " -a " + asc1 +
             " --headerline true " +
             " --fields='a int,b string'" ; 
  testRunCommand( command ) ;
  cmd.run( "rm -rf " + jsonfile ) ;
}

function testExprtImprtCsv(asc ,asc1)
{
   var csvfile = workDir + "sdbexprt15652.csv" ;
   cmd.run( "rm -rf " + csvfile ) ;
   println( "csv ascii for delchar is: " + asc ) ;
   var command = installPath + "bin/sdbexprt" +
                 " -s " + COORDHOSTNAME +
                 " -p " + COORDSVCNAME + 
                 " -c " + csname + 
                 " -l " + clname +
                 " --file " + csvfile + 
                 " --type csv" +
                 " -a " + asc + 
                 " --fields a,b" ;
   testRunCommand( command ) ;
   
   command = installPath + "bin/sdbimprt" +
             " -s " + COORDHOSTNAME +
             " -p " + COORDSVCNAME +
             " -c " + csname +
             " -l " + clname1 +
             " --file " + csvfile +
             " --type csv" +
             " -a " + asc1 +
             " --headerline true " +
             " --fields='a int,b string'" ; 
   testRunCommand( command ) ;
   cmd.run( "rm -rf " + csvfile ) ;
}
function check(){
	var cursor = cl1.find( {}, { _id: { $include: 0 } } ) ;
  var actRecs = getRecords( cursor ) ;
  checkRecords( expRecs, actRecs ) ;
   
  cl1.truncate() ;
}