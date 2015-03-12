// list cs.
// normal case.

function listCSAndCheck( db, csNames )
{
   try
   {
      for ( var i = 0; i < csNames.length; i++  )
      {
      	 var cur = db.exec("list collectionspaces") ;
         var found = false ;
         while(cur.next())
         {
         	if ( csNames[i] == cur.current().toObj()["Name"] )
            {
               found = true ;
               break ;
            }
         }
         if ( !found )
         {
            println( "can not find cs in list, cs: " + csNames[i] ) ;
            throw -1 ;
         }
         else
         {
         	  println( "find cs in list succ, cs: " + csNames[i] ) ;
         }
         cur.close();
      }
   }
   catch ( e )
   {
      throw e ;
   }
}
CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CS1 = CSPREFIX+"foo1" ;

CSPREFIX_CL = CSPREFIX+"bar" ; 

var db = new SecureSdb(COORDHOSTNAME, COORDSVCNAME) ;
var csNames = new Array( CSPREFIX_CS, CSPREFIX_CS1 ) ;

try
{
   db.execUpdate("drop collectionspace "+csNames[0]) ;
   db.execUpdate("drop collectionspace "+csNames[1]) ;
   //db.dropCS( csNames[1]) ;
}
catch ( e )
{
		if(e!=-34){
		pringln("failed to drop cs..rc="+e);
		throw e ;
	}
}

try
{
   db.execUpdate("create collectionspace "+csNames[0] ) ;
}
catch ( e )
{
	 //collection space already exist
   if(e!=-33){
   	   println( "failed to create cs " + csNames[0] +  ", rc= " + e )
   		 throw e ;
  }
}

try
{
   db.execUpdate("create collectionspace "+csNames[1] ) ;
}
catch ( e )
{
	 //collection space already exist
   if(e!=-33){
   	   println( "failed to create cs " + csNames[1] +  ", rc= " + e )
   		 throw e ;
  }
}

try
{
   listCSAndCheck( db, csNames ) ;
}
catch ( e )
{
   println( "test case returns error" ) ;
   throw e ;
}

try
{
   //db.dropCS( csNames[1]) ;
   db.execUpdate("drop collectionspace "+csNames[1]) ;
}
catch ( e )
{
   println( "failed to drop cs, rc= " + e ) ;
   throw e ;
}

csNames.pop() ;

try
{
   listCSAndCheck( db, csNames ) ;
}
catch ( e )
{
   println( "test case returns error" ) ;
   throw e ;
}

try
{
   //db.dropCS( csNames[0]) ;
   db.execUpdate("drop collectionspace "+csNames[0]) ;
   
}
catch ( e )
{
   println( "failed to drop cs, rc= " + e ) ;
   throw e ;
}

