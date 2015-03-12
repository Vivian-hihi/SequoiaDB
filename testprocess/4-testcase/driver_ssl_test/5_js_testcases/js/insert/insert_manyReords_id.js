// insert record.
// normal case.
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ; 
var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   if ( e != -34)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ; 
try{

var claSize = new RSize( CSPREFIX_CS );

var varCS = db.createCS(CSPREFIX_CS);

var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()},{Compressed:true});

}catch( e ){
   throw e ;	
}

try
{
   varCL.insert([{"_id":"10000a",str:"abcz",integer:1000,boolean1:true, boolean2:false, nullobj:null,user:{id:0, name:"name"},array:[{name:"qiu",balance:1.2}, {name:"shang", balance:-1.2}]},{"_id":"120000a",no:1002,score:85,interest:["movie","photo"],major:"计算机软件与理论",dep:"计算机学院",info:{name:"Holiday",age:22,sex:">女"}},{"_id":"13000a","°′″＄￡￥‰％℃¤￠":"○一二三四五六七八九百千万亿兆吉太拍艾分厘毫微零壹贰叁肆伍陆柒捌玖佰仟"}]) ;
}
catch ( e )
{
   println( "failed to insert record, rc= " + e ) ;
   throw e ;
}
var rc ;


try
{
   rc = varCL.find() ; 
}
catch ( e )
{
   println( "failed to read record, rc= " + e ) ;
   throw e ;
}
var cl = rc ;

try
{
	var clArr = new Array() ;
	while (cl.next())
	{
		clArr.push(cl.current().toObj()["_id"]) ;
	}	
	println(clArr) ;	
	if (clArr[0]!="10000a" && clArr[1]!="120000a" && clArr[2]!="13000a")
	{
		throw -2 ;
	}	
}
catch ( e )
{
	println("Failed to insert Data "+e+"+"+clArr.length) ;
	throw e ; 

}

println("Success to insert records") ;
try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to drop cs, rc= " + e ) ;
   throw e ;
}
