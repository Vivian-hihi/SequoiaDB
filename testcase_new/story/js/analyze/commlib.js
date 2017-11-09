/************************************
*@Description: 插入记录,数据页超过10页
*@author:      zhaoyu
*@createDate:  2017.11.8
**************************************/
function insertDatas( dbcl, insertNum )
{  
   try
   {
      //插入不同记录
      var doc = [];
      for(var i=0;i<insertNum;i++)
      {
         doc.push({a:i,a0:i,a1:i,a2:i,a3:i,a4:i,a5:i,a6:i,a7:i,a8:i,a9:i,
                   a10:i,a11:i,a12:i,a13:i,a14:i,a15:i,a16:i,a17:i,a18:i,a19:i,
                   a20:i,a21:i,a22:i,a23:i,a24:i,a25:i,a26:i,a27:i,a28:i,a29:i,
                   a30:i,a31:i,a32:i,a33:i,a34:i,a35:i,a36:i,a37:i,a38:i,a39:i,
                   a40:i,a41:i,a42:i,a43:i,a44:i,a45:i,a46:i,a47:i,a48:i,a49:i,
                   a50:i,a51:i,a52:i,a53:i,a54:i,a55:i,a56:i,a57:i,a58:i,a59:i,
                   a60:i,a61:i,a62:i,a63:i,a64:i,a65:i,a66:i,a67:i,a68:i,a69:i});
      }
      dbcl.insert(doc);
      
      //插入相同的记录
      var doc = [];
      for(var i=0;i<insertNum;i++)
      {
         doc.push({a:9000,a0:9000,a1:9000,a2:9000,a3:9000,a4:9000,a5:9000,a6:9000,a7:9000,a8:9000,a9:9000,
                   a10:9000,a11:9000,a12:9000,a13:9000,a14:9000,a15:9000,a16:9000,a17:9000,a18:9000,a19:9000,
                   a20:9000,a21:9000,a22:9000,a23:9000,a24:9000,a25:9000,a26:9000,a27:9000,a28:9000,a29:9000,
                   a30:9000,a31:9000,a32:9000,a33:9000,a34:9000,a35:9000,a36:9000,a37:9000,a38:9000,a39:9000,
                   a40:9000,a41:9000,a42:9000,a43:9000,a44:9000,a45:9000,a46:9000,a47:9000,a48:9000,a49:9000,
                   a50:9000,a51:9000,a52:9000,a53:9000,a54:9000,a55:9000,a56:9000,a57:9000,a58:9000,a59:9000,
                   a60:9000,a61:9000,a62:9000,a63:9000,a64:9000,a65:9000,a66:9000,a67:9000,a68:9000,a69:9000});
      }
      dbcl.insert(doc);
   }
   catch(e)
   {
      throw buildException("insertDatas()", e, "insert", "insert success", e);
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
   
   //比较各节点lsn
   for(var j=0; j< LSNs.length -1; j++)
   {
      if(LSNs[j] === LSNs[j+1])
      {
         var checkLSN = true;
      }else
      {
         var checkLSN = false;
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
function checkExplain( db, csName, clName, findConf, sortConf, hintConf, expExplains )
{
   if ( typeof(findConf) == "undefined" ) { findConf = null; }
   if ( typeof(sortConf) == "undefined" ) { sortConf = null; }
   if ( typeof(hintConf) == "undefined" ) { hintConf = null; }
   var rc = db.getCS(csName).getCL(clName).find(findConf).sort(sortConf).hint(hintConf).explain({Run:true}).toArray();
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