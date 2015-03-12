//CSPREFIX = "suse";
CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"student" ;//student table
CSPREFIX_CL1 = CSPREFIX+"grade" ;  //grade table

var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
//clear environment
try
{
   db.execUpdate( "drop collectionspace "+CSPREFIX_CS ) ;
}
catch (e)
{
   if ( e != -34)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}
//create CS
try 
{
	db.execUpdate("create collectionspace "+CSPREFIX_CS) ;
}
catch( e )
{
	println ("failed to create CS , rc1 = "+e);
	throw e ;
}
//create student table.
try
{
  //db.execUpdate("create collection "+CSPREFIX_CS+"."+CSPREFIX_CL);	
  var claSize = new RSize( CSPREFIX_CS );
  var varCS = db.getCS(CSPREFIX_CS);
  var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()});
}
catch( e )
{
	println ("failed to create student table , rc = "+e);
	throw e ;
}
//create grade table
try
{
  db.execUpdate("create collection "+CSPREFIX_CS+"."+CSPREFIX_CL1);	
}
catch( e )
{
	println ("failed to create grade table , rc = "+e);
	throw e ;
}
//insert 5 records to student table
var name =  new Array("Tom","Mike","John","Lily","Lucy") ;
var dept = new Array("computer","physics","mathematics","chemistry");

var _dept;
for(var i=0;i<name.length;i++){
	if(i==0 || i==1) _dept=dept[0];
	else _dept = dept[i-1];
		//db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(Sno,Sname) values"+"("+i+",\""+name[i]+"\")");
	try
	{
		db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(Sno,Sname,Sdept) values"+"("+i+",\""+name[i]+"\",\""+_dept+"\")");
	}
	catch(e)
	{
		println("failed to insert records to student table , rc ="+e);
		throw e ;
	}
}
//insert 5 records to grade table
var cno = new Array("1001","1002","1003","1004","1005","1006","1007");
var grade =  new Array("96","85","90","89","86","83","95");
for(var j=0;j<cno.length;j++){
	if (j%2==0) sno=j;
	else sno =1 ;
	try
	{
		db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL1+"(Sno,Cno,Grade) values"+"("+sno+","+cno[j]+","+grade[j]+")");
	}
	catch(e)
	{
		println("failed to insert records to grade table , rc ="+e);
		throw e ;
	}
}
//use join to select records

try
{
	var cur = db.exec("select s.Sname,s.Sdept,g.Cno,g.Grade from "+CSPREFIX_CS+"."+CSPREFIX_CL+" as s"+" inner join "+CSPREFIX_CS+"."+CSPREFIX_CL1+" as g"+" on s.Sno=g.Sno order by g.Cno /*+use_hash()*/");
}
catch(e)
{
	println("err happend when use join,rc="+e);
	throw e ;
}

var cno = new Array();
var grade = new Array();
var sname =  new Array();
var sdept =  new Array();
var bool = true ;
while(cur.next()){
	
	sname[i]=cur.current().toObj()["Sname"];
	grade[i]=cur.current().toObj()["Grade"];
	cno[i]=cur.current().toObj()["Cno"];
	sdept[i]=cur.current().toObj()["Sdept"];
	if(cno[i]==null||grade[i]==null||sdept[i]==null||sname[i]==null)
		{
			bool=false ;
			break ;
		}
}

if(!bool)
{
	println("failed to execte join");
	throw -2;
	}

try
{
	db.execUpdate("drop collectionspace "+CSPREFIX_CS);
}
catch(e)
{
  println("failed to drop cs;rc="+e);
  throw e ;	
}
