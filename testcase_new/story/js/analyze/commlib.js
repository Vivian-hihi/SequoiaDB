/************************************
*@Description: 插入20000条记录,数据页超过10页
*@author:      zhaoyu
*@createDate:  2017.11.8
**************************************/
function insertDatas( dbcl )
{  
   try
   {
      //插入10000条不同记录
      for(var j=0;j<1;j++)
      {
         var doc = [];
         for(var i=0;i<10000;i++)
         {
            doc.push({a:j*10000+i,b:j*10000+i,c:"test"+j*10000+i});
         }
         dbcl.insert(doc);
      }
      //插入10000条相同的记录
      for(var j=0;j<1;j++)
      {
         var doc = [];
         for(var i=0;i<10000;i++)
         {
            doc.push({a:9000,b:9000,c:"test"+9000});
         }
         dbcl.insert(doc);
      }
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
*@Description: 检查统计信息
*@author:      zhaoyu
*@createDate:  2017.11.8
**************************************/
function checkStat( db, csName, clName, indexName, clExistStat, indexExistStat )
{
   if( clExistStat == undefined ){ clExistStat = true;}
   if( indexExistStat == undefined ){ indexExistStat = true;}
   var groups = commGetCLGroups( db, csName + "." + clName );
   for(var i= 0; i< groups.length; i++)
   {
      var rg = db.getRG(groups[i]);
      var rgDetail = eval( "(" + rg.getDetail().toArray()[0] + ")");
      var nodesInGroup = rgDetail.Group;
      for(var j= 0; j< nodesInGroup.length; j++)
      {
         var clStatFlag = false;
         var indexStatFlag = false;
         var hostName = nodesInGroup[j].HostName;
         var serviceName = nodesInGroup[j].Service[0].Name;
         var data = new Sdb(hostName, serviceName);
         var clStats = data.SYSSTAT.SYSCOLLECTIONSTAT.find().toArray();
         if(clExistStat === true && clStats.length <1)
         {
            throw "NO_CL_STAT";
         } 
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
            
         if((clExistStat ^ clStatFlag) === 1)
         {
            println("clExistStat:" + clExistStat);
            println("clStatFlag:" + clStatFlag);
            throw "NO_CL_STAT";
         } 
         
         var indexStats = data.SYSSTAT.SYSINDEXSTAT.find().toArray();
         if(indexExistStat === true && indexStats.length <1)
         {
            throw "NO_INDEX_STAT";
         }
         for(var h= 0; h< indexStats.length; h++ )
         {
            var indexStat = eval( "(" + indexStats[h] + ")");
            var actualCSName = indexStat.CollectionSpace;
            var actualCLName = indexStat.Collection;
            var actualIndexName = indexStat.Index;
            if(actualCSName === csName && 
               actualCLName === clName &&  
               actualIndexName === indexName )
            {
               indexStatFlag = true;
            }
         }
          
         if((indexExistStat ^ indexStatFlag) === 1)
         {
            println("indexExistStat:" + indexExistStat);
            println("indexStatFlag:" + indexStatFlag);
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