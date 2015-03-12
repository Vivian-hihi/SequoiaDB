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
   varCL.insert([{str:"abcz",integer:1000,boolean1:true, boolean2:false, nullobj:null,user:{id:0, name:"name"},array:[{name:"qiu",balance:1.2}, {name:"shang", balance:-1.2}]},{no:1002,score:85,interest:["movie","photo"],major:"计算机软件与理论",dep:"计算机学院",info:{name:"Holiday",age:22,sex:">女"}},{"°′″＄￡￥‰％℃¤￠":"○一二三四五六七八九百千万亿兆吉太拍艾分厘毫微零壹贰叁肆伍陆柒捌玖佰仟"}]) ;
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

rc = rc.toArray();
if( 3 != rc.length ){
  	println("Failed to find records "+rc.length) ; 
	throw -1 ;
}
println("Success to insert many records") ;
try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to drop cs, rc= " + e ) ;
   throw e ;
}
