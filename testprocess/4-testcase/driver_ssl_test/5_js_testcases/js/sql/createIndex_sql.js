
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
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
//create CL.
try
{
  //db.execUpdate("create collection "+CSPREFIX_CS+"."+CSPREFIX_CL);	
   var claSize = new RSize( CSPREFIX_CS );
   var varCS = db.getCS(CSPREFIX_CS);
   var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()});
}
catch( e )
{
	println ("failed to create CL , rc2 = "+e);
	throw e ;
}
//insert 20 records
for ( var i = 0 ; i<20 ; i++){
	try{
		db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+" (name,age) values (\"Tom\","+i+")");
		}
		catch(e){
			println("failed to insert records , rc ="+e);
			throw e ;
			}
	}
//drop index which is not exist
var res = false ;
try
{
	db.execUpdate("drop index index_inexist on "+CSPREFIX_CS+"."+CSPREFIX_CL);
}
catch(e)
{
  if(e==-47)
  res = true ;
}
if(!res)
{
	throw -1 ;	
}

//ceate index for the field "age" and desc
try
{
	db.execUpdate("create index index_age on "+CSPREFIX_CS+"."+CSPREFIX_CL+" (age desc)");
}
catch(e)
{
  println("failed to create index_age,rc="+e);
  throw e ;	
}
//create the same indexname again 
var res = false ;
try
{
	db.execUpdate("create index index_age on "+CSPREFIX_CS+"."+CSPREFIX_CL+" (age desc)");
}
catch(e)
{
  if(e==-247)
  res = true ;	
}
if(!res)
{
	throw -2 ;	
}
try
{
	db.execUpdate("drop index index_age on "+CSPREFIX_CS+"."+CSPREFIX_CL);
}
catch(e)
{
  println("failed to drop index_age,rc="+e);
  throw e ;	
}
//create index for at list two filed
try
{
	db.execUpdate("create index test_index on "+CSPREFIX_CS+"."+CSPREFIX_CL+" (age,name)");
}
catch(e)
{
  println("failed to create test_index,rc="+e);
  throw e ;	
}
try
{
	db.execUpdate("drop index test_index on "+CSPREFIX_CS+"."+CSPREFIX_CL);
}
catch(e)
{
  println("failed to drop test_index,rc="+e);
  throw e ;	
}
//create unique index for the field "name"
try
{
	db.execUpdate("create unique index index_unique on "+CSPREFIX_CS+"."+CSPREFIX_CL+" (age)");
}
catch(e)
{
  println("failed to create index_unique,rc="+e);
  throw e ;	
}
try
{
	db.execUpdate("drop index index_unique on "+CSPREFIX_CS+"."+CSPREFIX_CL);
}
catch(e)
{
  println("failed to drop index_unique,rc="+e);
  throw e ;	
}
//drop the index index_unique again
var res = false ;
try
{
	db.execUpdate("drop index index_unique on "+CSPREFIX_CS+"."+CSPREFIX_CL);
}
catch(e)
{
  if(e==-47)	
  res = true ;
}
if(!res)
{
	throw -3 ;
	}
//create a index which start with $
var res = false ;
try
{
	db.execUpdate("create index "+"$index_name "+ "on "+CSPREFIX_CS+"."+CSPREFIX_CL+" (name)");
}
catch(e)
{
  if( e==-6 )
  res = true ;	
}
if(!res)
{
	throw -4 ;
	}
//drop a index start with $
var res = false ;
try
{
  db.execUpdate("drop index $index_name on "+CSPREFIX_CS+"."+CSPREFIX_CL);
}
catch(e)
{
	if(e==-47)
	res = true ;
}
if(!res){
	throw -12;
	}
//create a index which contain .
var res = false ;
try
{
	db.execUpdate("create index "+"in.dex_name "+ "on "+CSPREFIX_CS+"."+CSPREFIX_CL+" (name)");
}
catch(e)
{
  if( e==-6 )
  res = true ;	
}
if(!res)
{
	throw -5 ;
	}
//drop a index contain "."
var res = false ;
try
{
  db.execUpdate("drop index in.dex_name on "+CSPREFIX_CS+"."+CSPREFIX_CL);
}
catch(e)
{
	if(e==-47)
	res = true ;
}
if(!res){
	throw -13;
	}
//create a index which is a empty string
var res = false ;
try
{
	db.execUpdate("create index "+" "+ "on "+CSPREFIX_CS+"."+CSPREFIX_CL+" (name)");
}
catch(e)
{
  if( e==-195 )
  res = true ;	
}
if(!res)
{
	throw -6 ;
	}
//drop a index which is a empty string 
var res = false ;
try
{
  db.execUpdate("drop index "+" "+" on "+CSPREFIX_CS+"."+CSPREFIX_CL);
}
catch(e)
{
	if(e==-195)
	res = true ;
}
if(!res){
	throw -14;
	}
//create index without field
var res = false ;
try
{
	db.execUpdate("create index test_inde on "+CSPREFIX_CS+"."+CSPREFIX_CL);
}
catch(e)
{
  if( e==-195 )
  res = true ;	
}
if(!res)
{
	throw -7 ;
}
//create index without index name
var res = false ;
try
{
	db.execUpdate("create index on "+CSPREFIX_CS+"."+CSPREFIX_CL+" (name)");
}
catch(e)
{
  if( e==-195 )
  res = true ;	
}
if(!res)
{
	throw -8 ;
}

//clear environment
try
{
   db.execUpdate( "drop collectionspace "+CSPREFIX_CS ) ;
}
catch (e)
{
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
}
