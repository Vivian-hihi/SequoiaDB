/************************************
*@Description：对range分区组进行范围切分，指定分区键为int类型
*@author：2016-3-11 wuyan  Init
**************************************/
var clName = CHANGEDPREFIX + "_intKey7479";
function main()
{	
	try
	{
	   //@ clean before
	   if( true == commIsStandalone( db ) )
      {
         println( "run mode is standalone" );
         return;
      }     
      //less two groups no split
      var allGroupName = getGroupName2(db,true);         
      if( 1 === allGroupName.length )
      {
         println("--least two groups");
         return ;
      } 
		var dbCL = createSplitCl( clName, {test:1},"range");
		//insert 100 datas
		try
      {  
         var rd = new commDataGenerator();
   		var recs = rd.getRecords(100,"int",['test']);		
         dbCL.insert( recs ) ;
      }
      catch ( e )
      {
         throw buildException("Failed to insert data", e );
      }
		//var insertNum = 100;
		//var splitKeys = insertStrData( dbCL, insertNum ); s
      
      var splitKeys = generateSplitKeys( rd, "int","test" )
      println("beginkey="+JSON.stringify(splitKeys[0]))  
      println("endkey="+JSON.stringify(splitKeys[1]))
      splitGrInfo = ClSplitOneTimes( COMMCSNAME, clName, splitKeys[0], splitKeys[1] );
       
	   //check the datas of the splitgroups,find a data in splitgroups and check count	
		checkSplitResult( clName, splitGrInfo, "test",splitKeys,recs );		
		 
		//@ clean end
		commDropCL( db, COMMCSNAME, clName, false, false,"drop CL in the beginning" );
   }
   catch(e)
   {
      throw e;
   }
   finally
   {
      if (undefined !== db)
      {
         db.close();
      }
   }
}
main();




   
   
