/************************************
*@Description: 64个索引执行统计
*@author:      zhaoyu
*@createdate:  2017.11.9
*@testlinkCase:seqDB-11406
**************************************/
function main()
{
   var clName = COMMCLNAME + "_11406";
   var insertNum = 2000;
	var sameValues = 9000;
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the beginning" ) ;
   
   //创建cl
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   
   //创建索引
   for(var i=0; i<63; i++)
   {
      var obj = {};
      obj["a" + i] = 1;
      commCreateIndex( dbcl, "a" + i, obj);
   }
   
   //插入记录
	insertDiffDatas( dbcl, insertNum );
	insertSameDatas( dbcl, insertNum, sameValues );
   
   //执行统计
   analyze( db, {Collection:COMMCSNAME + "." + clName} );
   
   //检查统计信息
   for(var i=0; i<63; i++)
   {
      checkStat( db, COMMCSNAME, clName, "a" + i, true, true );
   }
   
   //检查主备节点访问计划
   for(var i=0; i<63; i++)
   {
      var findConf= {};
      findConf["a" + i] = 9000;
      var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
      
      db.setSessionAttr( { PreferedInstance: "m" } );
      checkExplain( dbcl, findConf, null, null, expExplains );
      
      db.setSessionAttr( { PreferedInstance: "s" } );
      checkExplain( dbcl, findConf, null, null, expExplains ); 
   }
   println("check result after analyze success!");
   
   //删除索引
   for(var i=0; i<63; i++)
   {
      commDropIndex( dbcl, "a" + i );
   }
   
   //检查统计信息
   for(var i=0; i<64; i++)
   {
      checkStat( db, COMMCSNAME, clName, "a" + i, true, false );
   }
   
   //检查主备节点访问计划
   for(var i=0; i<64; i++)
   {
      var findConf= {};
      findConf["a" + i] = 9000;
      var expExplains = [{ScanType:"tbscan", IndexName:"", ReturnNum:insertNum}];
      
      db.setSessionAttr( { PreferedInstance: "m" } );
      checkExplain( dbcl, findConf, null, null, expExplains );
      
      db.setSessionAttr( { PreferedInstance: "s" } );
      checkExplain( dbcl, findConf, null, null, expExplains ); 
   }
   println("check result drop index success!");
   
}
main()
