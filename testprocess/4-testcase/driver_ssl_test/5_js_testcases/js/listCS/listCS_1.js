// list cs.
// normal case.

function listCSAndCheck( db, csNames )
{
   try
   {
      // 是不是确认自己建的CS在就可以了，如果前面用例跑失败了未清除
      // 按原来的逻辑会导致现在的用例失败
      var csnum =0;
      var indexAry = new Array(csNames.length);
      var index =0;
      var cur = db.listCollectionSpaces() ;
      while(cur.next())
      {
         //var found = false ;
         for ( var j = 0; j < csNames.length; j++ )
         {
            if ( csNames[j] == cur.current().toObj()["Name"] )
            {
               indexAry[index++]=j;
               csnum +=1;
               break ;
            }
         }
         if (csnum == csNames.length)
         {
            return;
         }
      }
      if ( csnum !=  csNames.length)
      {
      	 var found = false;
      	 for (var i=0; i< csNames.length; ++i)
      	 {
      	 	  for (var k=0; k < indexAry.length; ++k)
      	 	  {
      	 	  	if (i == indexAry[k])
      	 	  	{
      	 	  	   found = true;
      	 	  	   break;
      	 	  	}
      	 	  }
      	 	  
      	 	  if (!found)
      	 	  {
      	 	     println( "can not find cs in list, cs: " + csNames[i] ) ;
      	 	  }
         }
         throw -1 ;
      }
   }
   catch ( e )
   {
      throw e ;
   }
}
CSPREFIX_CS = COMMCSNAME ;
CSPREFIX_CS1 = COMMCSNAME + "1" ;

CSPREFIX_CL = COMMCLNAME ;

var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
var csNames = new Array( CSPREFIX_CS, CSPREFIX_CS1 ) ;

try
{
   db.dropCS( csNames[0]) ;
   db.dropCS( csNames[1]) ;
}
catch ( e )
{

}

try
{
   db.createCS( csNames[0] ) ;
   db.createCS( csNames[1] ) ;
}
catch ( e )
{
   println( "failed to create cs, rc= " + e )
   throw e ;
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
   db.dropCS( csNames[1]) ;
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
   db.dropCS( csNames[0]) ;
}
catch ( e )
{
   println( "failed to drop cs, rc= " + e ) ;
   throw e ;
}

