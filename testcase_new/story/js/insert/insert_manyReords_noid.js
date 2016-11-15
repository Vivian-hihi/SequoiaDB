// insert record.
// normal case.
try
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true, "drop cl in the beginning" ) ;
}
catch(e)
{
   println( "unexpected err happened when clear cs:" + e ) ;
   throw e ;
}

try{
   var claSize = new RSize( COMMCSNAME );
   var varCS = commCreateCS( db, COMMCSNAME, true, "create CS in the beginning" );
   var varCL = varCS.createCL(COMMCLNAME,{ReplSize:claSize.ReplSize(),
                                          Compressed:true});

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

try
{
   if (varCL.find().count() != 3)
   throw "number error";
}
catch (e)
{
   throw "find record failed";
}

try
{
var obj1 = {str:"abcz",integer:1000,boolean1:true, boolean2:false, nullobj:null,user:{id:0, name:"name"},array:[{name:"qiu",balance:1.2}, {name:"shang", balance:-1.2}]};
var obj2 = {no:1002,score:85,interest:["movie","photo"],major:"计算机软件与理论",dep:"计算机学院",info:{name:"Holiday",age:22,sex:">女"}};
var obj3 = {"°′″＄￡￥‰％℃¤￠":"○一二三四五六七八九百千万亿兆吉太拍艾分厘毫微零壹贰叁肆伍陆柒捌玖佰仟"};
var cur = varCL.find(obj1);
if (!compareObj(obj1, cur.next().toObj()))
   throw "obj1 compare failed"
var cur = varCL.find(obj2);
if (!compareObj(obj2, cur.next().toObj()))
   throw "obj2 compare failed"
var cur = varCL.find(obj3);
if (!compareObj(obj3, cur.next().toObj()))
   throw "obj3 compare failed"
cur.close();
}
catch (e)
{
   throw e;
}

try
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "drop colleciton in the end" );
}
catch ( e )
{
   println( "failed to drop cs, rc= " + e ) ;
   throw e ;
}
