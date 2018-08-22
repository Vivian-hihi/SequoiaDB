/***********************************************************************
* @Description : test export with -a random ascii
*                seqDB-15652:自定义任意16进制、10进制ascii码为字符串分隔符，导出到json文件
*								 seqDB-15654:自定义任意16进制、10进制ascii码为字符串分隔符，导出到csv文件         
* @author      : wangkexin
* 
************************************************************************/


   
main() ;

function main()
{
		var clname = COMMCLNAME + "_sdbexprt15652" ;
		var clname1 = COMMCLNAME + "_sdbimprt15652" ;
		var doc = { a: 1 , b: "exprtTest" } ;
		
		var cl = commCreateCL( db, COMMCSNAME, clname, 0 ) ;
		var cl1 = commCreateCL( db, COMMCSNAME, clname1, 0 ) ;
   	cl.insert( doc ) ; 
   
  	//JSON文件
  	//16进制与字母混合多字符分隔符
   	testExprtImprtJson("abc0x2A0x3f","abc*?",clname,clname1) ;
   	check(cl1)
   	//优化后可以尽量识别可识别的字符
   	testExprtImprtJson("0x2Aabc0x3f","'*abc?'",clname,clname1) ;
   	check(cl1)
   	//10进制ascii码和字母混合分隔符
   	testExprtImprtJson("\\\\107a","'ka'",clname,clname1) ;
   	check(cl1)
   	//10进制ascii码和数字混合分隔符（优化，尽量识别）
   	testExprtImprtJson("\\\\1071","'k1'",clname,clname1) ;
   	check(cl1)
   	//16进制分隔符
   	testExprtImprtJson("0x2A","'*'",clname,clname1) ;
   	check(cl1)
   	//不能识别的分隔符，保持原样
   	testExprtImprtJson("0xx","'0xx'",clname,clname1) ;
   	check(cl1)
   	//汉字分隔符
   	testExprtImprtJson("分隔符","'分隔符'",clname,clname1) ;
   	check(cl1)
   
   	//CSV文件
   	//16进制与字母混合多字符分隔符
   	testExprtImprtCsv("abc0x2A0x3f","abc*?",clname,clname1) ;
   	check(cl1)
   	//优化后可以尽量识别可识别的字符
   	testExprtImprtCsv("0x2Aabc0x3f","'*abc?'",clname,clname1) ;
   	check(cl1)
   	//10进制ascii码和字母混合分隔符
   	testExprtImprtCsv("\\\\107a","'ka'",clname,clname1) ;
   	check(cl1)
   	//10进制ascii码和数字混合分隔符（优化，尽量识别）
   	testExprtImprtCsv("\\\\1071","'k1'",clname,clname1) ;
   	check(cl1)
   	//16进制分隔符
   	testExprtImprtCsv("0x2A","'*'",clname,clname1) ;
   	check(cl1)
   	//不能识别的分隔符，保持原样
   	testExprtImprtCsv("0xx","'0xx'",clname,clname1) ;
   	check(cl1)
   	//汉字分隔符
   	testExprtImprtCsv("分隔符","'分隔符'",clname,clname1) ;
   	check(cl1)
   
   
   	commDropCL( db, COMMCSNAME, clname ) ;
   	commDropCL( db, COMMCSNAME, clname1 ) ;
}

function testExprtImprtJson(asc,asc1,clname,clname1)
{
   var jsonfile = workDir + "sdbexprt15652.json" ;
   cmd.run( "rm -rf " + jsonfile ) ;
   println( "json ascii for delchar is: " + asc ) ;
   var command = installPath + "bin/sdbexprt" +
                 " -s " + COORDHOSTNAME +
                 " -p " + COORDSVCNAME + 
                 " -c " + COMMCSNAME + 
                 " -l " + clname +
                 " --file " + jsonfile + 
                 " --type json" +
                 " -a " + asc + 
                 " --fields a,b" ;
  testRunCommand( command ) ;
   
  command = installPath + "bin/sdbimprt" +
             " -s " + COORDHOSTNAME +
             " -p " + COORDSVCNAME +
             " -c " + COMMCSNAME +
             " -l " + clname1 +
             " --file " + jsonfile +
             " --type json" +
             " -a " + asc1 +
             " --headerline true " +
             " --fields='a int,b string'" ; 
  testRunCommand( command ) ;
  cmd.run( "rm -rf " + jsonfile ) ;
}

function testExprtImprtCsv(asc ,asc1,clname,clname1)
{
   var csvfile = workDir + "sdbexprt15652.csv" ;
   cmd.run( "rm -rf " + csvfile ) ;
   println( "csv ascii for delchar is: " + asc ) ;
   var command = installPath + "bin/sdbexprt" +
                 " -s " + COORDHOSTNAME +
                 " -p " + COORDSVCNAME + 
                 " -c " + COMMCSNAME + 
                 " -l " + clname +
                 " --file " + csvfile + 
                 " --type csv" +
                 " -a " + asc + 
                 " --fields a,b" ;
   testRunCommand( command ) ;
   
   command = installPath + "bin/sdbimprt" +
             " -s " + COORDHOSTNAME +
             " -p " + COORDSVCNAME +
             " -c " + COMMCSNAME +
             " -l " + clname1 +
             " --file " + csvfile +
             " --type csv" +
             " -a " + asc1 +
             " --headerline true " +
             " --fields='a int,b string'" ; 
   testRunCommand( command ) ;
   cmd.run( "rm -rf " + csvfile ) ;
}
function check(cl1){
	var expRecs = [ "{\"a\":1,\"b\":\"exprtTest\"}" ] ;
	var cursor = cl1.find( {}, { _id: { $include: 0 } } ) ;
  var actRecs = getRecords( cursor ) ;
  checkRecords( expRecs, actRecs ) ;
   
  cl1.truncate() ;
}