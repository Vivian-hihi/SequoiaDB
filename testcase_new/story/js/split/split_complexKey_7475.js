/************************************
*@Description：对range分区组指定多个分区键进行范围切分，如指定3个分区键
*@author：2016-3-11 wuyan  Init
**************************************/
var clName = CHANGEDPREFIX + "_complexKey";
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
		var dbCL = createSplitCl( clName, {no1:1,no2:1,no3:1},"range");
		insertData( dbCL );		
		   
      var startCondition = { no1:0,no2:0,no3:0 };
      var endCondition   = { no1:10,no2:10,no3:10 }
      splitGrInfo = ClSplitOneTimes( COMMCSNAME, clName, startCondition, endCondition )
         
	   //check the datas of the splitgroups,find a data in splitgroups and check count	
		checkSplitResult( clName, splitGrInfo )
		 
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

function insertData(varCL)
{
   var doc = [];
   try
   {
      for(var i=-10;i<20;i++)
      {
         for(var j=-10;j<20;j++)
         {
            for(var k=-10;k<20;k++)
            {
               doc.push({no1:i,no2:j,no3:k});
            }
         }
      } 
      varCL.insert( doc ) ; 
   }
   catch ( e )
   {
      throw buildException("insertData()",null, "Failed to insert data ", "sucess", e);
   }
}
 //check the result of the split
function checkSplitResult( clName, dataNodeInfo )
{   
   try
   {
      var gdb = new Sdb(dataNodeInfo[0].HostName,dataNodeInfo[0].svcname); 
      var cl0 = gdb.getCS(COMMCSNAME).getCL(clName);       
      var srcNum = cl0.count(); 
      println("--the find count of "+dataNodeInfo[0].svcname+" is "+srcNum); 
      	
      var gdb = new Sdb(dataNodeInfo[1].HostName,dataNodeInfo[1].svcname); 
      var cl1 = gdb.getCS(COMMCSNAME).getCL(clName);       
      var tarNum = cl1.count(); 
      println("--the find count of "+dataNodeInfo[1].svcname+" is "+tarNum); 
      //check the split sections datas of the targetGroups 
      var tarNum1 = cl1.count({$and:[ {a:{$gt:0}} , {a:{$lt:10}} ]});
      var tarNum2 = cl1.count({$and:[ {a:{$et:0}} , {b:{$gt:0}} ]});
      var tarNum3 = cl1.count({$and:[ {a:{$et:0}} , {b:{$et:0}} , {c:{$gte:0}} ]});
      var tarNum4 = cl1.count({$and:[ {a:{$et:10}} , {b:{$lt:10}} ]});
      var tarNum5 = cl1.count({$and:[ {a:{$et:10}} , {b:{$et:10}}, {c:{$lt:10}} ]});
               
      while( Number(srcNum) !== 17690 && Number(tarNum) !== 9310 && sleepDuration < maxSleepDuration )
      {
         sleep( sleepInteval );
         sleepDuration += sleepInteval;                    
      }
	   if( Number(srcNum) !== 17690 && Number(tarNum) !== 9310 && Number(tarNum1) !== 8100 
                  && Number(tarNum2) !== 570 && Number(tarNum3) !== 20 && Number(tarNum4) !== 600
                  && Number(tarNum5) !== 20 && Number(tarNum6) !== 9310  )			
	   {
         throw buildException("checkSplitResult()", "count wrong");
         println("srcNum="+srcNum+","+"tarNum="+tarNum+","+"tarNum1="+tarNum1+","+"tarNum2="+tarNum2+","
                        +"tarNum3="+tarNum3+","+"tarNum4="+tarNum4+","+"tarNum5="+tarNum5);
	   }	
	}
	catch(e)
   {
      throw e;
   } 
	finally
   {
      if ( gdb !== undefined )
      {
         gdb.close();
      	gdb = undefined;
      } 
   }	      
} 	
      	
         
   
   
   