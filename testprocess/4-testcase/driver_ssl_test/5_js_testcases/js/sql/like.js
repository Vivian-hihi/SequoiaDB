//like
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ; 

var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME) ;
try
{
   db.execUpdate( "drop collectionspace "+CSPREFIX_CS ) ;
}
catch ( e )
{
   if ( e != -34)
   {
      println( "unexpected err happened when clear the foo:" + e ) ;
      throw e ;
   }
}

try
{
	db.execUpdate("create collectionspace "+CSPREFIX_CS);
}
catch(e)
{
  println("failed to create CS , rc =" +e);
  throw e ;	
}

	try 
{
	//db.execUpdate("create collection "+CSPREFIX_CS+"."+CSPREFIX_CL);
	var claSize = new RSize( CSPREFIX_CS );
  var varCS = db.getCS(CSPREFIX_CS);
  var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()});
}

catch(e)
{
	println("failed to create collection CL , rc = "+e);
	throw e ;
}
try
{
	
	varCL.insert({country:"china"});	
	varCL.insert({country:"USA"});	
	varCL.insert({country:"india"});	
	varCL.insert({country:"japan"});	
	varCL.insert({country:"korea"});	
	varCL.insert({country:"britain"});	
	varCL.insert({country:"germany"});	
	varCL.insert({country:"france"});	
	varCL.insert({country:"spain"});	
	varCL.insert({country:"russia"});	
	
}
catch(e)
{
	println("failed to insert records e="+e);
	throw e ;	
}
//  "\"匹配 :\p匹配字符"p"
try
{
	var rc = db.exec("select * from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where country like \"\p\"");
}
catch(e)
{
	println("failed to read record with the expression \p e="+e);
	throw e ;	
}

var record = new Array() ;
var i = 0 ;
while(rc.next())
{
	i++;
	record.push(rc.current().toObj()["country"]);	
}
if(!(record[0]=="japan"&&record[1]=="spain"))
{
	println("the information of the records is wrong,record[0]="+record[0]+"record[1]="+record[1]);
	throw -1;
}
if ( 2 != i )
{
   println( " get wrong number of records with the expression \p,rc.size="+i) ;
   throw -1 ;
}

//  "^"匹配 :^b表示以b开始的字符
try
{
	var rc = db.exec("select * from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where country like \"^b\"");
}
catch(e)
{
	println("failed to read record with the expression ^b e="+e);
	throw e ;	
}
var i =0 ;
var record = new Array() ;
while(rc.next())
{
	i++;
	record.push(rc.current().toObj()["country"]);	
}

if(!(record[0]=="britain"))
{
	println("the information of the records is wrong,record[0]="+record[0]);
	throw -1;
}
if ( 1 != i )
{
   println( " get wrong number of records with the expression \p,rc.size="+i) ;
   throw -1 ;
}
//  "$"匹配 :a$表示以a结尾的字符
try
{
	var rc = db.exec("select * from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where country like \"\ia$\"");
}
catch(e)
{
	println("failed to read record with the expression \ia$\ e="+e);
	throw e ;	
}
var i =0 ;
var record = new Array() ;
while(rc.next())
{
	i++ ;
	record.push(rc.current().toObj()["country"]);
}	

if(!(record[0]=="india"&&record[1]=="russia"))
{
	println("the information of the records is wrong,record[0]="+record[0]+"record[1]="+record[1]);
	throw -1;
}
if ( 2 != i )
{
   println( " get wrong number of records with the expression \ia$\,rc.size="+i) ;
   throw -1 ;
}

//  "*"匹配 :ia*表示匹配i或者ia零次或多次
try
{
	var rc = db.exec("select * from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where country like \"ia*\"");
}
catch(e)
{
	println("failed to read record with the expression \ia$\ e="+e);
	throw e ;	
}
var i =0 ;
var record = new Array() ;
while(rc.next())
{
	i++ ;
	record.push(rc.current().toObj()["country"]);
}	

if(!(record[0]="china"&&record[1]=="india"&&record[2]=="britain"&&record[3]=="spain"&&record[4]=="russia"))
{
	println("the information of the records is wrong");
	throw -1;
}
if ( 5 != i )
{
   println( " get wrong number of records with the expression \ia$\,rc.size="+i) ;
   throw -1 ;
}
//^b.*[st].*n$
try
{
	var rc = db.exec("select * from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where country like \"^b.*[st].*n$\"");
}
catch(e)
{
	println("failed to read record with the expression ^b.*[st].*n$ e="+e);
	throw e ;	
}
var i =0 ;
var record = new Array() ;
while(rc.next()){
	i++ ;
	record.push(rc.current().toObj()["country"]);
}

if(!(record[0]="britain"))
{
	println("the information of the records is wrong");
	throw -1;
}
if ( 1 != i )
{
   println( " get wrong number of records with the expression \ia$\,rc.size="+i) ;
   throw -1 ;
}
// [^a-t]
try
{
	var rc = db.exec("select * from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where country like \"[^a-t]\"");
}
catch(e)
{
	println("failed to read record with the expression [^a-t] e="+e);
	throw e ;	
}
var i =0 ;
var record = new Array() ;
while(rc.next())
{
	i++ ;
	record.push(rc.current().toObj()["country"]);
}	

if(!(record[0]="USA"&&record[1]=="germany"&&record[2]=="russia"))
{
	println("the information of the records is wrong");
	throw -1;
}
if ( 3 != i )
{
   println( " get wrong number of records with the expression [^a-t],rc.size="+i) ;
   throw -1 ;
}
// (an+|s{2})
try
{
	var rc = db.exec("select * from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where country like \"(an+|s{2})\"");
}
catch(e)
{
	println("failed to read record with the expression (an+|s{2}) e="+e);
	throw e ;	
}
var i=0;
var record = new Array() ;
while(rc.next())
{
	i++ ;
	record.push(rc.current().toObj()["country"]);
}	

if(!(record[0]="japan"&&record[1]=="germany"&&record[2]=="france"&&record[3]=="russia"))
{
	println("the information of the records is wrong");
	throw -1;
}
if ( 4 != i )
{
   println( " get wrong number of records with the expression (an+|s{2}),rc.size="+i) ;
   throw -1 ;
}
//clear environment
try
{
   db.execUpdate( "drop collectionspace "+CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to drop cs, rc= " + e ) ;
   throw e ;
}
