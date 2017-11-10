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
   
   //检查组内各节点cl统计表信息及索引统计表信息
   for(var i= 0; i< groups.length; i++)
   {
      //检查CL所在组的主备节点的lsn是否一致
      var lsnFlag = false;
      while(!lsnFlag)
      {
         lsnFlag = checkLSN(db, groups[i]);
      }
      
      var rg = db.getRG(groups[i]);
      var rgDetail = eval( "(" + rg.getDetail().toArray()[0] + ")");
      var nodesInGroup = rgDetail.Group;
      for(var j= 0; j< nodesInGroup.length; j++)
      {
         //检查cl统计表信息
         var clStatFlag = false;
         var hostName = nodesInGroup[j].HostName;
         var serviceName = nodesInGroup[j].Service[0].Name;
         var data = new Sdb(hostName, serviceName);
         var clStats = data.SYSSTAT.SYSCOLLECTIONSTAT.find().toArray();
         
         //需要检查cl统计表信息时，统计表信息不能为空
         if(clExistStat === true && clStats.length <1)
         {
            throw "NO_CL_STAT";
         }
         
         //cl统计表信息中存在统计信息且数据页不小于10
         for(var k= 0; k< clStats.length; k++ )
         {
            var clStat = eval( "(" + clStats[k] + ")");
            var actualCSName = clStat.CollectionSpace;
            var actualCLName = clStat.Collection;
            var totalDataPages = clStat.TotalDataPages;
            if( actualCSName === csName && 
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
         var indexStats = data.SYSSTAT.SYSINDEXSTAT.find().toArray();
         
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
*@Description: 检查访问计划
*@author:      zhaoyu
*@createDate:  2017.11.8
**************************************/
function checkExplain( dbcl, findConf, sortConf, hintConf, expExplains )
{
   if ( typeof(findConf) == "undefined" ) { findConf = null; }
   if ( typeof(sortConf) == "undefined" ) { sortConf = null; }
   if ( typeof(hintConf) == "undefined" ) { hintConf = null; }
	
	var rc = dbcl.find(findConf).sort(sortConf).hint(hintConf).explain({Run:true}).toArray();
   for(var i= 0; i< rc.length; i++)
   {
      var actExplain = eval("(" + rc[i] + ")");
      var expExplain = expExplains[i];
      for ( var f in expExplain )
   	{
   	   if((f == "ScanType") || (f == "IndexName") || (f == "ReturnNum"))
   	   {
   	      if( JSON.stringify(actExplain[f]) !== JSON.stringify(expExplain[f]) )
	   	   {
	   	      throw buildException("check explain", "CHECK_EXPLAIN_FAIL", "check explain failed!", 
	   		                     "\nactual value= " + JSON.stringify(actExplain[f]),
	   		                     "\nexpect value= "+JSON.stringify(expExplain[f]));
	   	   }
   	   }
   	}
   }
   
}
