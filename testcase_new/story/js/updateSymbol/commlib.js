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
function updateData( dbcl, updateCondition, findCondition )
{
   if ( typeof(findCondition) == "undefined" ) { findCondition = null ; }  
   try
   {
      dbcl.update( updateCondition, findCondition );
      println( "--update data success" ) ;
   }
   catch(e)
   {
      throw buildException("updateData()",e,"update", "update success","update fail");
   }
}

/************************************
*@Description: upsert data
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function upsertData( dbcl, upsertCondition, findCondition )
{
   if ( typeof(findCondition) == "undefined" ) { findCondition = null ; }  
   try
   {
      dbcl.upsert( upsertCondition, findCondition );
      println( "--upsert data success" ) ;
   }
   catch(e)
   {
      throw buildException("upsertData()",e,"upsert", "upsert success","upsert fail");
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
*@Description: get actual result and check it 
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function checkResult( dbcl, findCondition, findCondition2, expRecs, sortCondition )
{
   var rc = sortFindData(dbcl, findCondition, findCondition2, sortCondition );
   println("--begin to check the data");
   checkRec( rc, expRecs );
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
   
   //check every records every fields,expRecs as compare source
   for( var i in expRecs )
   {
   	var actRec = actRecs[i];
   	var expRec = expRecs[i];
   	
   	for ( var f in expRec )
   	{
   		if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) )
	   	{
	   		println("\nerror occurs in "+(parseInt(i)+1)+"th record, in field '"+f+"'");
	   		println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nexpect recs= "+JSON.stringify(expRecs));   		
	   		throw buildException("checkRec()", "rec ERROR");
	   	}
   	}
   }
   //check every records every fields,actRecs as compare source
   for( var i in actRecs )
   {
   	var actRec = actRecs[i];
   	var expRec = expRecs[i];
   	
   	for ( var f in actRec )
   	{
   	   if(f == "_id")
   	   {
   	      continue;
   	   }
   		if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) )
	   	{
	   		println("\nerror occurs in "+(parseInt(i)+1)+"th record, in field '"+f+"'");
	   		println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nexpect recs= "+JSON.stringify(expRecs));   		
	   		throw buildException("checkRec()", "rec ERROR");
	   	}
   	}
   }
}

/************************************
*@Description: check result when the expect result of update data is failed.
*@author:      zhaoyu 
*@createDate:  2016/5/16
*@parameters:               
**************************************/
function invalidDataUpdateCheckResult( dbcl, invalidDoc, expRecs )
{
   try
   {
      dbcl.update( invalidDoc );
      throw "need throw error";
   }
   catch(e)
   {
      if(expRecs != e )
      {
         throw buildException("invalidDataUpdateCheckResult() "+e+"\ncheckResult "+" \nExpect result: "+expRecs+" \nActual result: "+e);
      }
      else
   	{
   	    println("--check result is ok!");   		
   	}
   
   }
}

/************************************
*@Description: get group name and service name .
*@author:      wuyan 
*@createDate:  2015/10/20
*@parameters:               
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
*@Description: create index
*@author:      liuxiaoxuan
*@createDate:  2017.09.18
**************************************/
function createIndex( dbcl, indexName, key )
{
  try
  {
     dbcl.createIndex(indexName , key);
     println("--create index success");
   }
  catch(e)
   {
     throw buildException("createIndex()",e,"create index", "create success","create fail");
   }
}
