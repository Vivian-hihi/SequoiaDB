/************************************
*@Description: insert data
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function insertData( dbcl, condition )
{  
   try
   {
      dbcl.insert(condition);
      println( "--insert data success" ) ;
   }
   catch(e)
   {
      throw buildException("insertData()",e,"insert", "insert success","insert fail");
   }
}

/************************************
*@Description: update data
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function updateData( dbcl, condition )
{  
   try
   {
      dbcl.update( condition );
      println( "--update data success" ) ;
   }
   catch(e)
   {
      throw buildException("updateData()",e,"update", "update success","update fail");
   }
}

/************************************
*@Description: delete data
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function deleteData( dbcl, condition )
{
	 if ( typeof(condition) == "undefined" ) { condition = null ; }  
   try
   {
      dbcl.remove(condition);
      println( "--remove data success" ) ;
   }
   catch(e)
   {
      throw buildException("deleteData()",e,"delete", "delete success","delete fail");
   }
}

/************************************
*@Description: find and sort data
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function sortFindData(dbcl, findCondition, findCondition2, sortCondition )
{
	if ( typeof( findCondition ) == "undefined" ) { findCondition = null; }
   try
   {
      var sortResult = dbcl.find( findCondition, findCondition2 ).sort( sortCondition );
   }
   catch(e)
   {
      throw buildException("sortFindData()",e,"find and sort data", "find and sort data success","find and sort data fail");
   }
   return sortResult;
}

/************************************
*@Description: aggregate data
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function aggregate(dbcl, condition )
{
   try
   {
      var aggregateResult = dbcl.aggregate( condition ); 
   }
   catch(e)
   {
      throw buildException("aggregate()",e,"aggregate", "aggregate success","aggregate fail");
   }
   return aggregateResult;
}

/************************************
*@Description: aggregate data and check result
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function aggregateCheckResult( dbcl, condition, expRecs )
{
   var rc = aggregate(dbcl, condition );
   println("--begin to check the data");
   checkRec( rc, expRecs );
   println("--end check the data");
}

/************************************
*@Description: find data and
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function findData( dbcl, condition, condition2 )
{
	 if ( typeof(condition2) == "undefined" ) { condition2 = null; }
   try
   {
      var actualResault = dbcl.find( condition, condition2 ); 
      println( "--find data success" ) ;     
   }
   catch(e)
   {
      throw buildException("findData()",e,"findData", "findData success","findData fail");
   }
   return actualResault;
}

/************************************
*@Description: find data use sql,get actual result and check it 
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function checkSqlResult( db, sql, expRecs )
{
   var rc = db.exec( sql );
   
   println("--begin to check the data");
   checkRec( rc, expRecs );
   println("--end check the data");
}

/************************************
*@Description: get actual result and check it 
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function checkResult( dbcl, findCondition, findCondition2, expRecs, sortCondition )
{
	if( typeof(findCondition) == "undefined" ) { findCondition = null; }
	if( typeof(findCondition2) == "undefined" ) { findCondition2 = null; }
	if( typeof(expRecs) == "undefined" ) { expRecs = findCondition2; }
   //var rc = findData( dbcl, condition, condition2 );
   var rc = sortFindData(dbcl, findCondition, findCondition2, sortCondition )
   // when expRecs is JSON
   if ( expRecs == findCondition2 )
   {
      var expRecsArr = [];
      expRecsArr.push(expRecs);	
   }
   // when expRecs is Arr
   else
   {
      var expRecsArr = expRecs;
   }
   
   println("--begin to check the data");
   checkRec( rc, expRecsArr );
   println("--end check the data");
}

/************************************
*@Description: compare actual and expect result,
               they is not the same ,return error ,
               else return ok
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function checkRec( rc, expRecs )
{				
	//get actual records to array
	var actRecs = [];
   while( rc.next() )
   {
		actRecs.push( rc.current().toObj() );
   }
   //check count
	if( actRecs.length !== expRecs.length )
   {
   	println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nexpect recs= "+JSON.stringify(expRecs));
   	throw buildException("check count", null, "",
									expRecs.length, actRecs.length);
   }
   
   //check every records every fields
   for( var i in expRecs )
   {
   	var actRec = actRecs[i];
   	var expRec = expRecs[i];
   	
   	for ( var f in expRec )
   	{
   		if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) )
	   	{
	   		println("\nerror occurs in "+(parseInt(i)+1)+"th record, in field '"+f+"'");
			println("\nactual recs in cl= "+JSON.stringify(actRec)+"\n\nexpect recs= "+JSON.stringify(expRec));
	   		println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nexpect recs= "+JSON.stringify(expRecs));   		
	   		throw buildException("checkRec()", "rec ERROR");
	   	}
   	}
   }
}

/************************************
*@Description: get the informations of the srcGroups and targetGroups,then split cl with different options,
               only split 1 times
               return the informations of the srcGroups and targetGroups
*@author:      wuyan
*@createdate:  2015.10.14
**************************************/
function ClSplitOneTimes( csName, clName, startCondition, endCondition )
{
	try
	{
	   var targetGroupNums = 1;
      var groupsInfo = getSplitGroups(csName,clName,targetGroupNums);
      var srcGrName = groupsInfo[0].GroupName;    
      var tarGrName = groupsInfo[1].GroupName;
      var CL = db.getCS(csName).getCL(clName);
      println("--begin split") 
		if ( typeof(startCondition) === "number" ) //percentage split
		{
			CL.split( srcGrName, tarGrName, startCondition );
		}
		else if ( typeof(startCondition) === "object" && endCondition === undefined ) //range split without end condition
		{
			CL.split( srcGrName, tarGrName, startCondition );
			println("startCondition="+startCondition)
		}
		else if ( typeof(startCondition) === "object" && typeof(endCondition) === "object" ) //range split with end condition
		{
			CL.split( srcGrName, tarGrName, startCondition, endCondition );
		}	
		println("--end split")
	}
	catch ( e )
	{
		throw e;
	}
	return groupsInfo;
}
/************************************
*@Description: get Group name and Service name
*@author：wuyan 2015/10/20
**************************************/
function getGroupName(db, mustBePrimary)
{
   var RGname = null ;
   try
   {
      RGname = db.listReplicaGroups().toArray();
   }
   catch (e)
   {
      throw e;
   }
   var j = 0;
   var arrGroupName = Array();
   for (var i=1 ; i != RGname.length ; ++i )
   {
      var eRGname = eval('('+RGname[i]+')') ;   
      if( 1000 <= eRGname["GroupID"] )
      {
         arrGroupName[j] = Array();
         var primaryNodeID = eRGname["PrimaryNode"] ;
         var groups = eRGname["Group"] ;
         for ( var m = 0; m < groups.length; m++ )
         {  
            if ( true == mustBePrimary )
            {
               var nodeID = groups[m]["NodeID"] ;
               if ( primaryNodeID != nodeID )
                  continue ;
            }               
            arrGroupName[j].push(eRGname["GroupName"]) ;
            arrGroupName[j].push(groups[m]["HostName"]) ;
            arrGroupName[j].push(groups[m]["Service"][0]["Name"]) ;
            break ;
         }
         ++j;
      }
   }
   return arrGroupName;
}

/************************************
*@Description: get SrcGroup and TargetGroup info,the groups information
               include GroupName,HostName and svcname
*@author:      wuyan
*@createdate:  2015.10.14
**************************************/
function getSplitGroups(csName,clName,targetGrMaxNums)
{
   var allGroupInfo =  getGroupName(db, true);    
   var srcGroupName = getSrcGroup(csName, clName );
   var splitGroups = new Array();
   if( targetGrMaxNums >= allGroupInfo.length-1 ) 
   {
      targetGrMaxNums = allGroupInfo.length-1;      
   }
   var index =1;
   
   for( var i = 0 ; i != allGroupInfo.length ; ++i )
   {
      if( srcGroupName == allGroupInfo[i][0] )
      {
         splitGroups[0] = new Object();
			splitGroups[0].GroupName = allGroupInfo[i][0];
			splitGroups[0].HostName = allGroupInfo[i][1];
			splitGroups[0].svcname = allGroupInfo[i][2];	
      }
      else 
      {
         if (index > targetGrMaxNums)
         {
            continue;
         }      
         splitGroups[index] = new Object();
			splitGroups[index].GroupName = allGroupInfo[i][0];
			splitGroups[index].HostName = allGroupInfo[i][1];
			splitGroups[index].svcname = allGroupInfo[i][2];			          
         index++;                  
      }
   }   
   return splitGroups;
   
}

/************************************
*@Description: get SrcGroup name,update getPG to getSrcGroup
*@author:      wuyan
*@createdate:  2015.10.14
**************************************/
function getSrcGroup( csName, clName )
{
   try
   {
      if( undefined == csName || undefined == clName )
      {
         println( "cs name: " + csName + ", clName: " + clName ) ;
         throw "cs or cl name is undefined" ;
      }
      var tableName = csName + "." + clName ;
      var cataMaster = db.getCatalogRG().getMaster().toString().split(":");
      var catadb = new Sdb( cataMaster[0],cataMaster[1] ) ;
      var Group = catadb.SYSCAT.SYSCOLLECTIONS.find().toArray() ;
      var srcGroupName ;
      for( var i = 0 ; i < Group.length ; ++i )
      {
         var eachID = eval("("+Group[i]+")") ;
         if( tableName == eachID["Name"] )
         {
            srcGroupName = eachID["CataInfo"][0]["GroupName"] ;
            println( csName + "." + clName + "'s source group: " + srcGroupName );
            break ;
         }
      }
      return srcGroupName ;
   }
   catch( e )
   {
      println( "failed to get source group, cs name: " + csName +
               ", cl name: " + clName ) ;
      throw e ;
   }
}

/************************************
*@Description: get the actual result from src group and des group,
               get the expect result from the function argument,
               then check result.
*@author:      zhaoyu 
*@createDate:  2016/4/28
*@parameters:  dataNodeInfo:[object1,object2,.....]
               expRecs:[[object1,object2,.....],[object3,object4,.....]]               
**************************************/
function checkRangeClSplitResult( db, clName, dataNodeInfo, findCondition, findCondition2, expRecs, sortCondition )
{  
   //get data from src and des groups
   for( var i = 0 ; i < dataNodeInfo.length ; i++ )
   {            
      try
      {
         //get data from function argument
         var expGroupRecs = expRecs[i];
         
         //get data from master node
         //var getMasterNode= db.getRG(dataNodeInfo[i].GroupName).getMaster().toString().split(":");
         //var gdb = new Sdb(getMasterNode[0],getMasterNode[1]);
         var gdb = new Sdb(dataNodeInfo[i].HostName,dataNodeInfo[i].svcname); 
         var cl = gdb.getCS(COMMCSNAME).getCL(clName);
         var sortRec = sortFindData( cl, findCondition, findCondition2, sortCondition );
         
         //check data
         println("--start to check the "+i+"th data");
         checkRec( sortRec, expGroupRecs );
         println("--end check the "+i+"th data");
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
}


function checkHashClSplitResult( db, clName, dataNodeInfo, findCondition, findCondition2, expRecs, sortCondition )
{  
   //get data from src and des groups
   var actRecs =new Array();
   for( var i = 0 ; i < dataNodeInfo.length ; i++ )
   {            
      try
      {
         //get data from function argument
         var expGroupRecs = expRecs[i];
         
         //get data from master node
         var gdb = new Sdb(dataNodeInfo[i].HostName,dataNodeInfo[i].svcname); 
         var cl = gdb.getCS(COMMCSNAME).getCL(clName);
         var sortRec = sortFindData( cl, findCondition, findCondition2, sortCondition );
         
         //get actual records to array
	      actRecs[i]= new Array();
         while( sortRec.next() )
         {
            actRecs[i].push( sortRec.current().toObj() );
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
   
   //check data
   println("--start to check the data");
   //check count
   arrLength = 0;
   for(var j = 0;j < actRecs.length; j++)
   {
      arrLength += actRecs[j].length; 
   } 
	if( arrLength !== expRecs )
   {
      println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nactual recs length = "+arrLength+"\n\nexpect recs length = "+expRecs);
   	throw buildException("check count", null, "",
									expRecs, arrLength);
   }
   //check every records every fields
   for(var s in actRecs[0])
   {
      for(var x in actRecs[1])
      {
         //println("-----actRecs[0]"+JSON.stringify(actRecs[0][s]));
         //println("=====actRecs[1]"+JSON.stringify(actRecs[1][x]));
         if(actRecs[0][s] == actRecs[1][x])
         {
            println("two group exist the same data:"+JSON.stringify(actRecs[0][s]));
   	      throw buildException("checkRec()", "rec ERROR");    
         }      
      }   
   }
   println("--end check the data");        
}

/************************************
*@Description: check result when the expect result of find data is failed.
*@author:      zhaoyu 
*@createDate:  2016/4/28
*@parameters:               
**************************************/
function InvalidArgCheck( dbcl, condition, condition2, expRecs )
{ 
   try
   {
      dbcl.find( condition, condition2 ).toArray();   
      throw "need throw error";
  
   }
   catch(e)
   { 
     if(expRecs != e )
   	 {
		    throw buildException("InvalidArgCheck() "+e+"\ncheckResult "+" \nExpect result: "+expRecs+" \nActual result:"+e);   
   	 }
   	 else
   	 {
   	    println("check result is ok!");   		
   	 } 
   }
}

/************************************
*@Description: attach cl.
*@author:      zhaoyu 
*@createDate:  2016/4/28
*@parameters:               
**************************************/
function attachCL( dbcl, subCLName, attachOption )
{
   try
   {
      println( "--Begin to attach cl" ) ;
      dbcl.attachCL( subCLName, attachOption );
   }
   catch(e)
   {
      throw buildException("attachCL()",e,"attachCL", "attachCL success","attachCL fail");
   }
}

/************************************
*@Description: detach cl.
*@author:      zhaoyu 
*@createDate:  2016/4/28
*@parameters:               
**************************************/
function detachCL( dbcl, subCLName )
{
   try
   {
      println( "--Begin to detach cl" ) ;
      dbcl.detachCL( subCLName );
   }
   catch(e)
   {
      throw buildException("detachCL()",e,"detachCL", "detachCL success","detachCL fail");
   }
}

/************************************
*@Description: check result when the expect result of insert data is failed.
*@author:      zhaoyu 
*@createDate:  2016/4/28
*@parameters:               
**************************************/
function invalidDataInsertCheckResult( dbcl, invalidDoc, expRecs )
{
   try
   {
      dbcl.insert( invalidDoc );
      throw "need throw error";
   }
   catch(e)
   {
      if(expRecs != e )
      {
         throw buildException("invalidDataInsertCheckResult() "+e+"\ncheckResult "+" \nExpect result: "+expRecs+"\nActual result: "+e);
      }
      else
   	{
   	    println("check result is ok!");   		
   	}
   
   }
}

/************************************
*@Description: check test environment before split.
*@author:      zhaoyu 
*@createDate:  2016/5/23
*@parameters:               
**************************************/
function checkConditionBeforeSplit( db )
{
   try
	{
	   //standalone can not split
	   if( true == commIsStandalone( db ) )
      {
         println( "run mode is standalone" );
         return;
      }     
      //less two groups,can not split
      var allGroupName = getGroupName( db );       
      if( 1 === allGroupName.length )
      {
         println("--least two groups");
         return ;
      }
   }
   catch( e )
   {
      throw e;
      }
}
