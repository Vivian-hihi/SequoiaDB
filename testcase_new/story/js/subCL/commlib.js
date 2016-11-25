/* *****************************************************************************
@Description: subCL common functions
@modify list:
   2014-07-30 pusheng Ding  Init
   2015-12-19 Ting YU modify
***************************************************************************** */

//2015-12-19 Ting YU modify
function getSourceGroupName_alone( csName , clName )
{
   var clFullName = csName + "." + clName;
   var clInfo = db.snapshot( 8, {Name: clFullName} );
   while( clInfo.next() )
   {
      var clInfoObj = clInfo.current().toObj();
      var GroupName = clInfoObj.CataInfo[0].GroupName;
   }
   return GroupName;
}
function getOtherDataGroups( SourceGroupName )
{
	var allGroups =db.listReplicaGroups().toArray() ;
	var RoleGroupNumbers = 0 ;
	var Groups = [];
	for ( var i = 0; i<allGroups.length; i++ )
	{
		var eval_node=eval("("+allGroups[i]+")");
		if(eval_node["Role"]==0 && eval_node["GroupID"] >= 1000 )
		{
			if( eval_node["GroupName"] != SourceGroupName )
			{
				Groups.push( eval_node["GroupName"] );
			}
		}
	}
	return Groups;
}

//2015-12-19 Ting YU modify
function getPartition( csName ,clName )
{
   var clFullName = csName + "." + clName;
   var clInfo = db.snapshot( 8, {Name: clFullName} );
   while( clInfo.next() )
   {
      var clInfoObj = clInfo.current().toObj();
      var Partition = clInfoObj.Partition;
   }
	return Partition ;
}
function subCL_split_hash( subcl, SourceGroupName, OtherDataGroups, Partition)
{
	var Partition_PerGroup = Partition / ( OtherDataGroups.length + 1 ) ;
	for( var i = 0; i < OtherDataGroups.length; ++i )
	{
		var start_Partition = Math.round( Partition_PerGroup * i ) ;
		var end_Partition = Math.round( Partition_PerGroup * ( i + 1 ) ) ;
		println( start_Partition + '~~~~~~~~~~~~~~~~' + end_Partition ) ;
		try
		{
			subcl.split( SourceGroupName, OtherDataGroups[i], {Partition:start_Partition}, {Partition:end_Partition} ) ;
		}
		catch( e )
		{
			println( "can't split : " + e ) ;
			return -1
		}
	}
	return 0 ;
}

/************************************
*@Description: compare actual and expect result,
               they is not the same ,return error ,
               else return ok
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function checkRec( rc, expectRecordNum, sortOptions )
{			
	//get actual records to array
	var actRecs = [];
   while( rc.next() )
   {
		actRecs.push( rc.current().toObj() );
   }
   //check count
	if( actRecs.length !== expectRecordNum )
   {
   	println("\nactual recs in cl= "+ JSON.stringify(actRecs));
   	throw buildException("check count", null, "", actRecs.length, expectRecordNum);
   }
   
   //check every records every fields,expRecs as compare source
   var actRecCurrent = actRecs[0];
   for( var i = 1; i< actRecs.length; i++ )
   {
   	var actRecNext = actRecs[i];
   	if(compareJSONObj(actRecCurrent, actRecNext, sortOptions) < 0)
   	{
   	   println("\nerror occurs in " + (parseInt(i) + 1) + "th record");
	   	println("\ncurrent record= "+JSON.stringify(actRecs[i])+"\n\nnext record= "+JSON.stringify(actRecs[i+1]));
	   	throw buildException("checkRec()", "rec ERROR");  
   	}
   	actRecCurrent = actRecNext;
   }
   println( "--check data success" ) ;	
}

// compareJSONObj(compareObj,toCompareObj,{a:1,b:-1,c:1})
function compareJSONObj(objA, objB, options) {
	var optionKeys = [];
	for (var key in options) {
		optionKeys.push(key);
	}
	for (var i = 0; i < optionKeys.length; i++) {
		var sortFlag = parseInt(eval("options."+optionKeys[i]));
		var aValue = eval("objA."+optionKeys[i]);
		var bValue = eval("objB."+optionKeys[i]);
		if (aValue === undefined)  aValue = bValue - 1;
		if (bValue === undefined)  bValue = aValue - 1;
		if (aValue === undefined && bValue === undefined) { aValue = 0; bValue = 0; }
		if (sortFlag === 1) {
			//ÕýÐò
			if (aValue < bValue) {
				return 1;
			} else if(aValue > bValue) {
				return -1;
			}
		} else {
			//ÄæÐò
			if (aValue > bValue) {
				return 1;
			} else if(aValue < bValue) {
				return -1;
			}			
		}
	}
	return 0;
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
      println(csName+"."+clName+"'s target group: "+tarGrName);
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
*@author£ºwuyan 2015/10/20
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
*@Description: attach cl
*@author:      zhaoyu
*@createdate:  2016.11.23
**************************************/
function attachCL( dbcl, subCLName, range )
{
   try
   {
      dbcl.attachCL( subCLName, range ) ;
      println( "--attach cl success" ) ;
   }
   catch(e)
   {
      throw buildException("attachCL()", e, "attach cl", "attach cl success","attach cl fail");
   }
}

/************************************
*@Description: bulk insert data
*@author:      zhaoyu
*@createDate:  2016.11.23
**************************************/
function insertBulkData( dbcl, recordNum, recordStart, recordEnd )
{
   try
   {
      var doc = [];
   	for(var i=0;i<recordNum;i++)
   	{
         var aValue = recordStart + parseInt( Math.random() * (recordEnd-recordStart) );
   	   var bValue = recordStart + parseInt( Math.random() * (recordEnd-recordStart) );
   	   var cValue = recordStart + parseInt( Math.random() * (recordEnd-recordStart) );
   	   doc.push({a:aValue,b:bValue,c:cValue}); 
   	}
   	dbcl.insert(doc);
      println( "--bulk insert data success" ) ;
   }
   catch(e)
   {
      throw buildException("insertBulkData()", e, "insert", "insert data :" + JSON.stringify(doc), "insert fail");
   }
   return doc;
}

