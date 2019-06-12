/* *****************************************************************************
@discretion: cl asynchronous split , check the task id
@author：2018-11-06 wangkexin
***************************************************************************** */

main(db);
function main(db)
{	  
	try
	{	
	   if (commGetGroupsNum(db) < 2)
      {
         println("--least two groups");
         return ;
      }   

	  var clName = CHANGEDPREFIX + "_checktaskidcl16328";     
      commDropCL( db, COMMCSNAME, clName, true, true, "clear collection in the beginning" ) ; 
      var options = {ShardingKey:{No:1}, ShardingType:"range",ReplSize:0,Compressed:true};
      
      var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, options, true, true );
      var recordNums = 30000;      
      insertData( db, COMMCSNAME, clName, recordNums ); 
         
      var targetGroupNums = 2;
      var groupsInfo = getSplitGroups( COMMCSNAME, clName, targetGroupNums );
      var taskId = splitCL( COMMCSNAME, clName, groupsInfo);      
      
      println("---Begin to check result ");
	  checkTaskId( taskId );
      checkSplitResult( COMMCSNAME, clName, recordNums, groupsInfo);     
      
      commDropCL( db, COMMCSNAME, clName, true, true, "clear collection in the ending" ) ;      
   }
   catch( e )
   {
      throw e ;
   }   
}

function splitCL( csName, clName, groupsInfo )
{
   try
   {
      println("---Begin to splitAsync"); 
      var dbcl = db.getCS( csName ).getCL( clName );             
      var percent = 90;
      var srcGroupName = groupsInfo[0].GroupName;
      var dstGroupName = groupsInfo[1].GroupName;      
      var taskId = dbcl.splitAsync(srcGroupName, dstGroupName, percent); 
      return taskId;
   } 
   catch( e )
   {
      throw buildException("splitAsync datas", e)
   } 
}

function checkTaskId( taskId )
{
	try
   {
	   println("---Begin to check the task id");
	   var resultArr = db.listTasks({"TaskID":taskId}).toArray();
	   if(resultArr.length !== 1)
	   {
		   throw buildException("check listTasks()", null, "check the task id",
									taskId, "not exist");
		}
   }
   catch(e)
   {
      throw buildException("check task id", e)
   } 
}

function checkSplitResult( csName, clName, expRecordNums, groupsInfo )
{   
   try
   {  println("---Begin to check the split result");
	  //waiting for split 
      var sleepInteval=10;
      var sleepDuration=0;
      var maxSleepDuration=120000;
	  
      while( (db.listTasks({ "Name": csName + "."+ clName }).next() !== undefined ) && sleepDuration < maxSleepDuration )
      {        
         sleep( sleepInteval );
         sleepDuration += sleepInteval;                       
      }
      
	  //check the record nums      
      var dbcl = db.getCS( csName ).getCL( clName );
      var count = dbcl.count();      
      if( count != expRecordNums )
      {
         throw buildException("check datas", null, "check the new cl record nums, sleepDuration : " + sleepDuration,
									expRecordNums, count);
      }   
       
      //test record nums of split groups
      for( var i = 0; i < 2; i++ )
      {         
         try
         {
            var sdb = new Sdb(groupsInfo[i].HostName,groupsInfo[i].svcname);            
            var cl = sdb.getCS( csName ).getCL( clName );
            var num = cl.count();  
            println(num)       
			
            if( i == 1 && Number(num) !== expRecordNums*0.9 )			
	         {  
               throw buildException("checkClSplitRecordNums", "count wrong", "count()",expRecordNums*0.9, num)
	         }	      
         }
         catch(e)
         {
            throw buildException("checkClSplitResult()", e);
         }
         finally
         {
            if (sdb !== undefined)
      	   {
               sdb.close();      	      
      	   } 
         }	           
      }    
   }
   catch(e)
   {
      throw buildException("check split", e)
   }  
}
