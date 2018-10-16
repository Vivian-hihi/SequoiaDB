/************************************
*@Description: insert data
*@author:      wuyan
*@createDate:  2018.1.22
**************************************/
function insertData( dbcl, number)
{
   if( undefined == this.number ){ this.number = 1000 ; }
   try
   {
      println("---Begin to insert data " );   
      var docs = [];
      for( var i = 0; i < number; ++i )
      {      
         var no = i;
         var a = i;
         var user = "test"+i;
         var phone = 13700000000+i;
         var time = new Date().getTime(); 
         var doc = {no:no, a:a,customerName:user, phone:phone, openDate:time};      
         //data example: {"no":5, customerName:"test5", "phone":13700000005, "openDate":1402990912105
         
         docs.push( doc );
      }	
      dbcl.insert( docs );       
   }
   catch(e)
   {
      throw buildException("insertData()",e,"insert", "insert success","insert fail");
   }
}

function beginTrans( db )
{
   try
   {
      println( "---transBegin" );
      db.transBegin();     
      
   }
   catch( e )
   {
      throw buildException("beginTrans()", e);
   }
}


function commitTrans( db )
{
   try
   {
      println( "---transCommit" ) ;
      db.transCommit() ;
   }
   catch( e )
   {
      throw buildException("commitTrans()", e);
   }
}

function rollbackTrans()
{
   try
   {
      println( "--transRollback" ) ;
      db.transRollback() ;
   }
   catch( e )
   {
      throw buildException("rollbackTrans()", e );
   }
}

/************************************
*@Description: check the new cl name 
*@author:      wuyan
*@createDate:  2018.10.12
**************************************/
function checkRenameCLResult( csName, oldCLName, newCLName)
{   
   try
   {
      var clFullName = csName + "." + newCLName; 
      var getNewCLName = db.snapshot(SDB_SNAP_COLLECTIONS ,{"Name": clFullName }).current().toObj().Name;     
      if( getNewCLName !== clFullName  )
      {
         throw buildException("check cl name", null, "check the new cl name",
									clFullName, getNewCLName);
      }   
      
      //check the old cl is not exist
      try
	   {
		   db.getCS(csName).getCL( oldCLName );
		   throw "need throw error";
	   }
	   catch ( e )
	   { 
		   if ( e !== -23  )
		   {		      
			   throw buildException("check old clName:",e);
		   }		
	   }
   }
   catch(e)
   {      
      throw buildException("checkRenameCLResult", e)
   }   
}


