/************************************
*@Description: 插入不同记录,数据页超过10页
*@author:      zhaoyu
*@createDate:  2017.11.8
**************************************/
function insertDiffDatas( dbcl, insertNum )
{  
   try
   {
      //插入不同记录
      var doc = [];
      for(var i = 0; i < insertNum; i++)
      {
         doc.push({a:i,a0:i,a1:i,a2:i,a3:i,a4:i,a5:i,a6:i,a7:i,a8:i,a9:i,
                   a10:i,a11:i,a12:i,a13:i,a14:i,a15:i,a16:i,a17:i,a18:i,a19:i,
                   a20:i,a21:i,a22:i,a23:i,a24:i,a25:i,a26:i,a27:i,a28:i,a29:i,
                   a30:i,a31:i,a32:i,a33:i,a34:i,a35:i,a36:i,a37:i,a38:i,a39:i,
                   a40:i,a41:i,a42:i,a43:i,a44:i,a45:i,a46:i,a47:i,a48:i,a49:i,
                   a50:i,a51:i,a52:i,a53:i,a54:i,a55:i,a56:i,a57:i,a58:i,a59:i,
                   a60:i,a61:i,a62:i,a63:i,a64:i,a65:i,a66:i,a67:i,a68:i,a69:i,
						 b:i,c:"test" + i});
      }
      dbcl.insert(doc);
   }
   catch(e)
   {
      throw buildException("insertDiffDatas()", e, "insert", "insert success", e);
   }
}

/************************************
*@Description: 插入相同记录,数据页超过10页
*@author:      zhaoyu
*@createDate:  2017.11.8
**************************************/
function insertSameDatas( dbcl, insertNum, value )
{  
   try
   {
      //插入相同的记录
      var doc = [];
      for(var i = 0;i < insertNum; i++)
      {
         doc.push({a:value,a0:value,a1:value,a2:value,a3:value,a4:value,a5:value,a6:value,a7:value,a8:value,a9:value,
                   a10:value,a11:value,a12:value,a13:value,a14:value,a15:value,a16:value,a17:value,a18:value,a19:value,
                   a20:value,a21:value,a22:value,a23:value,a24:value,a25:value,a26:value,a27:value,a28:value,a29:value,
                   a30:value,a31:value,a32:value,a33:value,a34:value,a35:value,a36:value,a37:value,a38:value,a39:value,
                   a40:value,a41:value,a42:value,a43:value,a44:value,a45:value,a46:value,a47:value,a48:value,a49:value,
                   a50:value,a51:value,a52:value,a53:value,a54:value,a55:value,a56:value,a57:value,a58:value,a59:value,
                   a60:value,a61:value,a62:value,a63:value,a64:value,a65:value,a66:value,a67:value,a68:value,a69:value,
						 b:value,c:"test" + value});
      }
      dbcl.insert(doc);
   }
   catch(e)
   {
      throw buildException("insertSameDatas()", e, "insert", "insert success", e);
   }
}

/************************************
*@Description: 执行统计
*@author:      zhaoyu
*@createDate:  2017.11.8
**************************************/
function analyze( db, options )
{
   if ( typeof(options) == "undefined" ) { options = null; }
   try
   {
      db.analyze( options );
   }catch(e)
   {
      throw buildException("analyze()", e, "analyze", "analyze success", e);
   }
}

/************************************
*@Description: 检查主备节点lsn是否一致
*@author:      zhaoyu
*@createDate:  2017.11.8
**************************************/
function checkLSN(db, group)
{
   var rg = db.getRG(group);
   var rgDetail = eval( "(" + rg.getDetail().toArray()[0] + ")");
   var nodesInGroup = rgDetail.Group;
   var LSNs = new Array();
   for(var j= 0; j< nodesInGroup.length; j++)
   {
      //获取各节点lsn
      var clStatFlag = false;
      var hostName = nodesInGroup[j].HostName;
      var serviceName = nodesInGroup[j].Service[0].Name;
      var data = new Sdb(hostName, serviceName);
      var getSnapshot6 = eval( "(" + data.snapshot(6).toArray()[0] + ")" );
      var currentLSN = getSnapshot6.CurrentLSN.Offset;
      LSNs.push(currentLSN);
   }
   
   var checkLSN = true;

   //比较各节点lsn
   for(var j=0; j< LSNs.length -1; j++)
   {
      if(LSNs[j] === LSNs[j+1])
      {
         checkLSN = true;
      }else
      {
         checkLSN = false;
         break;
      }
   }
   return checkLSN;
}

/************************************
*@Description: 检查统计信息
*@author:      zhaoyu
*@createDate:  2017.11.8
**************************************/
function checkStat( db, csName, clName, indexName, clExistStat, indexExistStat )
{
   if( clExistStat == undefined ){ clExistStat = true;}
   if( indexExistStat == undefined ){ indexExistStat = true;}
   var groups = commGetCLGroups( db, csName + "." + clName );
   
   var dataDB = new Array();
   
   //get all nodes
   if(0 == groups.length)
   {
      dataDB[0] = Array();
      dataDB[0][0] = db;
   }
   else
   {
      for(var i = 0; i< groups.length; i++)
      {
         var lsnFlag = false;
         while(!lsnFlag)
         {
            lsnFlag = checkLSN(db, groups[i]);
            //println("check primary and slave node lsn flag:" + lsnFlag);
            sleep(500);
         }
         
         var rg = db.getRG(groups[i]);
         var rgDetail = eval( "(" + rg.getDetail().toArray()[0] + ")");
         var nodesInGroup = rgDetail.Group;
         
         dataDB[i] = Array();
         for(var j = 0; j < nodesInGroup.length; j++)
         {
            var hostName = nodesInGroup[j].HostName;
            var serviceName = nodesInGroup[j].Service[0].Name;
            dataDB[i][j] = new Sdb(hostName, serviceName);
         }
      }
   }
   
   //check each node  
   for(var i = 0; i< dataDB.length; i++)
   {
      for(var j = 0; j< dataDB[i].length; j++)
      {
         //检查cl统计表信息
         var clStatFlag = false;   
         var clStats = dataDB[i][j].SYSSTAT.SYSCOLLECTIONSTAT.find().toArray();
         
         //需要检查cl统计表信息时，统计表信息不能为空
         if(clExistStat === true && clStats.length <1)
         {
            throw "NO_CL_STAT";
         }
         
         //cl统计表信息中存在统计信息且数据页不小于10
         for(var k = 0; k< clStats.length; k++ )
         {
            var clStat = eval( "(" + clStats[k] + ")");
            var actualCSName = clStat.CollectionSpace;
            var actualCLName = clStat.Collection;
            var totalDataPages = clStat.TotalDataPages;
            if(actualCSName === csName && 
               actualCLName === clName &&
               totalDataPages > 10 )
            {
               clStatFlag = true;
            }
         }
         
         //是否存在cl统计表信息与预期结果校验   
         if((clExistStat ^ clStatFlag) === 1)
         {
            println("hostName:" + hostName + "\nserviceName:" + serviceName + "\nclExistStat:" + clExistStat + "\nclStatFlag:" + clStatFlag);
            throw "NO_CL_STAT";
         } 
         
         //检查索引统计表信息
         var indexStatFlag = false;
         var indexStats = dataDB[i][j].SYSSTAT.SYSINDEXSTAT.find().toArray();
         
         //需要检查索引统计表信息时，统计表信息不能为空
         if(indexExistStat === true && indexStats.length <1)
         {
            throw "NO_INDEX_STAT";
         }
         
         //索引统计表信息中存在统计信息
         for(var h= 0; h< indexStats.length; h++ )
         {
            var indexStat = eval( "(" + indexStats[h] + ")");
            var actualCSName = indexStat.CollectionSpace;
            var actualCLName = indexStat.Collection;
            var actualIndexName = indexStat.Index;
            if(actualCSName === csName && 
               actualCLName === clName &&  
               actualIndexName === indexName &&
               indexStat.hasOwnProperty('MCV'))
            {
               indexStatFlag = true;
            }
         }
         
         //是否存在索引统计表信息与预期结果校验  
         if((indexExistStat ^ indexStatFlag) === 1)
         {
            println("hostName:" + hostName + "\nserviceName:" + serviceName + "\nindexExistStat:" + indexExistStat + "\nindexStatFlag:" + indexStatFlag);
            throw "NO_INDEX_STAT";
         }  
      }
   }
     
}

/************************************
*@Description: 按组获取表访问计划
*@author:      zhaoyu
*@createDate:  2017.11.10
**************************************/
function getCommonExplain( dbcl, findConf, sortConf, hintConf )
{
   if ( typeof(findConf) == "undefined" ) { findConf = null; }
   if ( typeof(sortConf) == "undefined" ) { sortConf = null; }
   if ( typeof(hintConf) == "undefined" ) { hintConf = null; }
   
   //保存所有访问计划
   var explains = new Array();
   var rc = dbcl.find(findConf).sort(sortConf).hint(hintConf).explain({Run:true}).toArray();
   var groupExplains = eval("(" + rc + ")");
   var explainObj = {};
   for( var f in groupExplains )
	{
	   if((f == "ScanType") || (f == "IndexName") || (f == "ReturnNum"))
	   {
	      explainObj[f] = groupExplains[f];     
	   }
	}
	explains.push(explainObj);
   return explains;
   
}

/************************************
*@Description: 按组获取表访问计划
*@author:      zhaoyu
*@createDate:  2017.11.10
**************************************/
function getSplitExplain( dbcl, findConf, sortConf, hintConf )
{
   if ( typeof(findConf) == "undefined" ) { findConf = null; }
   if ( typeof(sortConf) == "undefined" ) { sortConf = null; }
   if ( typeof(hintConf) == "undefined" ) { hintConf = null; }
   
   //保存所有访问计划
   var explains = new Array();
   var rc = dbcl.find(findConf).sort(sortConf).hint(hintConf).explain({Run:true}).toArray();
   for(var i= 0; i< rc.length; i++)
   {
      var groupExplains = eval("(" + rc[i] + ")");
      var explainObj = {};
      for( var f in groupExplains )
   	{
   	   if((f == "GroupName") || (f == "ScanType") || (f == "IndexName") || (f == "ReturnNum"))
   	   {
   	      explainObj[f] = groupExplains[f];     
   	   }
   	}
   	explains.push(explainObj);
   }
   return explains;
   
}

/************************************
*@Description: 校验访问计划
*@author:      zhaoyu
*@createDate:  2017.11.8
**************************************/
function checkExplain( actExplains, expExplains )
{
   //校验长度
   if(actExplains.length !== expExplains.length)
   {
      throw buildException("check count", "COUNT_ERR", "check count failed!",
									expExplains.length, actExplains.length);
   }
   
   //校验访问计划，不校验元素顺序
   for(var i=0; i< expExplains.length; i++)
   {
      if(JSON.stringify(actExplains).indexOf(JSON.stringify(expExplains[i])) === -1)
      {
         throw buildException("checkMainclExplain", "CHECK_EXPLAIN_FAIL", "check explain failed!", 
	   		                  JSON.stringify(expExplains[i]), JSON.stringify(actExplains));
      }
   }
   
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
*@Description: 手工修改索引统计表
*@author:      liuxiaoxuan
*@createDate:  2017.11.13
**************************************/
function updateIndexStateInfo( db, csName, clName, indexName, mcvValues, fracs )
{
   var dataDB= new Array();
   
   if(commIsStandalone(db))
   {
      dataDB[0] = Array();
      dataDB[0][0] = db;
   }
   else
   {
      var groupNames = commGetCLGroups( db, csName + "." + clName );
      var groupDetail = new Array();
      for(var i in groupNames)
      {
         dataDB[i] = Array();
         groupDetail = commGetGroups( db, false, groupNames[i] );
         for(var j = 1; j < groupDetail[0].length; j++)
         {
            var hostName = groupDetail[0][j].HostName;
            var svcName = groupDetail[0][j].svcname;
            dataDB[i][j-1] = new Sdb(hostName, svcName);
         }
      }
      
   }		
   
   for(var i in dataDB)
   {
      for(var j in dataDB[i])
      {
         try
         {
            var rec = dataDB[i][j].SYSSTAT.SYSINDEXSTAT.find().toArray();
                                                                                     
            if(0 < rec.length)
            {				 
               var rule = {"$set": {"MCV": {"Values": mcvValues, "Frac": fracs}}}; 
                                                                                        
               var matcher = {"$and": [{"CollectionSpace" : csName},
                                       {"Collection" : clName},
                                       {"Index" : indexName}]};
                                                                                     	
               dataDB[i][j].SYSSTAT.SYSINDEXSTAT.upsert(rule, matcher);
            }
         }
         catch(e)
         {
            throw buildException("modify SYSIndexInfo", e, "modify", "modify success", e);
         }	   
      }      
   }
                                                                                       
}                                                                                
