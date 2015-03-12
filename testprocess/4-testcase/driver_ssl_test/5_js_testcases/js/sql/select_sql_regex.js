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

try{
   db.execUpdate("insert into " + CSPREFIX_CS+"."+CSPREFIX_CL + " (name) values('Aom1') ");
   db.execUpdate("insert into " + CSPREFIX_CS+"."+CSPREFIX_CL + " (name) values('aom2') ");
   db.execUpdate("insert into " + CSPREFIX_CS+"."+CSPREFIX_CL + " (name) values('Bom3') ");
   db.execUpdate("insert into " + CSPREFIX_CS+"."+CSPREFIX_CL + " (name) values('Aom4') ");
   db.execUpdate("insert into " + CSPREFIX_CS+"."+CSPREFIX_CL + " (name) values('Tom1') ");
}catch( e ){
   println("error on insert the first data");
   throw e;	
}
try{
	
   var cur = db.exec("select * from " + CSPREFIX_CS + "." + CSPREFIX_CL +" where name like '^A.*1$' ");
   while( cur.next() ){
      if( "Aom1" != cur.current().toObj()["name"] )
         throw -1;	
   }
   
   var cur = db.exec("select * from " + CSPREFIX_CS + "." + CSPREFIX_CL +" where name like 'C+' ");
   while( cur.next() ){
      throw -2;	
   }
   
   var cur = db.exec("select * from " + CSPREFIX_CS + "." + CSPREFIX_CL +" where name like 'C*' ");
   i = 0;
   while( cur.next() ){
      i++;	
   }
   if( i != 5 )
      throw -3;
      
   var cur = db.exec("select * from " + CSPREFIX_CS + "." + CSPREFIX_CL +" where name like '[A-B]' ");
   i = 0;
   while( cur.next() ){
      i++;	
   }
   if( i != 3 )
      throw -4;
      
   var cur = db.exec("select * from " + CSPREFIX_CS + "." + CSPREFIX_CL +" where name like '.*m1{1,}' ");
   i = 0;
   while( cur.next() ){
      i++;	
   }
   if( i != 2 )
      throw -5;
	 
	 var cur = db.exec("select * from " + CSPREFIX_CS + "." + CSPREFIX_CL +" where name like 'Aom.' ");
   i = 0;
   while( cur.next() ){
      i++;	
   }
   if( i != 2 )
      throw -6;
      
   var cur = db.exec("select * from " + CSPREFIX_CS + "." + CSPREFIX_CL +" where name like 'B|T' ");
   i = 0;
   while( cur.next() ){
      i++;	
   }
   if( i != 2 )
      throw -7;
      
   var cur = db.exec("select * from " + CSPREFIX_CS + "." + CSPREFIX_CL +" where name like '^[aB]' ");
   i = 0;
   while( cur.next() ){
      i++;	
   }
   if( i != 2 )
      throw -8;
	 
	 var cur = db.exec("select * from " + CSPREFIX_CS + "." + CSPREFIX_CL +" where name like '^[^aAB]' ");
   i = 0;
   while( cur.next() ){
      i++;	
   }
   if( i != 1 )
      throw -9;
}catch( e ){
	println("sql regex rule error ");
	throw -e ;
}





//clear environment
try
{
  // db.execUpdate( "drop collectionspace "+CSPREFIX_CS ) ;
}
catch (e)
{
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e 
}
