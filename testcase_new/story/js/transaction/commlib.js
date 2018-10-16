/* *****************************************************************************
@Description: sdb transaction common function 
@modify list:
   2014-4-1 YiBang Ruan  Init
***************************************************************************** */

function dbNew( db )
{
   try
   {
      db = new Sdb( COORDHOSTNAME, COORDSVCNAME ) ;
   }
   catch( e )
   {
      println( " new  Sdb failed : " + e ) ;
      throw e ;
   }
}

function dbClose( db )
{
   try
   {
      db.close() ;
   }
   catch( e )
   {
      println( " close Sdb failed : " + e ) ;
      throw e ;
   }
}

function dbArrayNew( db )
{
   try
   {
      for( i = 0; i < CONNECTNUM; ++i )
      {
         db[i] = new Sdb( COORDHOSTNAME, COORDSVCNAME ) ;
      }
   }
   catch( e )
   {
      println( " new the " + i + "st Sdb failed : " + e ) ;
      throw e ;
   }
}

function dbArrayClose( db )
{
   try
   {
      for( i = 0; i < CONNECTNUM; ++i )
      {
         db[i].close() ;
      }
   }
   catch( e )
   {
      println( " close the" + i + "st Sdb failed : " + e ) ;
      throw e ;
   }
}

/************************************
*@Description: check the new cs name 
*@author:      luweikang
*@createDate:  2018.10.13
**************************************/
function checkRenameCSResult( oldCSName, newCSName, clNum)
{   
   try
   {
      var newCSObj = db.snapshot(SDB_SNAP_COLLECTIONSPACES ,{"Name": newCSName }).current().toObj();     
      var getNewCSName = newCSObj.Name;
      if( getNewCSName !== newCSName  )
      {
         throw buildException("check cs name", null, "check the new cs name",
									newCSName, getNewCSName);
      }
      
      var clArray = newCSObj.Collection;
      
      if(clNum != clArray.length){
         throw buildException("check cl num", null, "check the cs.cl num",
                              clNum, clArray.length);
      }
      
      for( i = 0; i< clArray.length; i++)
      {
         var csname = clArray[i].Name.split(".")[0];
         if( csname !== newCSName  )
         {
            throw buildException("check cs.cl name", null, "check the new cs name",
                              newCSName, csname);
         }
      }
      
      //check the old cl is not exist
      try
	   {
		   db.getCS(oldCSName);
		   throw "CS_IS_EXIT";
	   }
	   catch ( e )
	   { 
		   if ( e !== -34  )
		   {		      
			   throw buildException("check old csName:",e);
		   }		
	   }
   }
   catch(e)
   {      
      throw buildException("checkRenameCSResult", e)
   }   
}

