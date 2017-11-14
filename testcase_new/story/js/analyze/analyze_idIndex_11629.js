/************************************
*@Description: 指定id索引生成默认统计信息并修手工改统计信息再清空
*@author:      zhaoyu
*@createdate:  2017.11.13
*@testlinkCase:seqDB-11629
**************************************/
function main()
{
   var clName = COMMCLNAME + "_11629";
   var insertNum = 4000;
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the beginning" ) ;
   
   //创建cl
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   
   //插入记录
	insertDatas( dbcl, insertNum );
	
   //执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName, Index:"$id"} );
   
   //获取主备节点
   db.setSessionAttr( { PreferedInstance: "m" } );
   var dbclPrimary = db.getCS(COMMCSNAME).getCL(clName);
   db.setSessionAttr( { PreferedInstance: "s" } );
   var dbclSlave = db.getCS(COMMCSNAME).getCL(clName);
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "$id", true, true );
   
   //检查主备节点访问计划
   var findConf = {_id:2000};
   var expExplains = [{ScanType:"ixscan", IndexName:"$id", ReturnNum:1}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after analyze success!");
   
   //生成默认统计信息
   analyze( db, {Mode:3, Collection: COMMCSNAME + "." + clName, Index:"$id"} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "$id", true, false );
   
   //检查主备节点访问计划
   var findConf = {_id:2000};
   var expExplains = [{ScanType:"ixscan", IndexName:"$id", ReturnNum:1}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result analyze mode set 3 success!");
   
   //手工修改主节点统计信息
   var mcvValues = [{_id:8000},{_id:9000},{_id:9001}];
   var fracs = [50,5000,50];
   updateIndexStateInfo( db, COMMCSNAME, clName, "$id", mcvValues, fracs );
   
   //统计信息加载至缓存
   analyze( db, {Mode:4, Collection: COMMCSNAME + "." + clName, Index:"$id"} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "$id", true, true );
   
   //检查主备节点访问计划
   var findConf = {_id:2000};
   var expExplains = [{ScanType:"ixscan", IndexName:"$id", ReturnNum:1}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result analyze mode set 4 success!");
   
   //清空统计信息
   analyze( db, {Mode:5, Collection: COMMCSNAME + "." + clName, Index:"$id"} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "$id", true, true );
   
   //检查主备节点访问计划
   var findConf = {_id:2000};
   var expExplains = [{ScanType:"ixscan", IndexName:"$id", ReturnNum:1}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result analyze mode set 5 success!");
   
   //再次更新统计信息
   var mcvValues = [{_id:8000},{_id:9000},{_id:9001}];
   var fracs = [50,50,50];
   updateIndexStateInfo( db, COMMCSNAME, clName, "$id", mcvValues, fracs );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "$id", true, true );
   
   //检查主备节点访问计划
   var findConf = {_id:2000};
   var expExplains = [{ScanType:"ixscan", IndexName:"$id", ReturnNum:1}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after update index stat but no analyze success!");
   
   //再次清空缓存
   analyze( db, {Mode:5, Collection: COMMCSNAME + "." + clName, Index:"$id"} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "$id", true, true );
   
   //检查主备节点访问计划
   var findConf = {_id:2000};
   var expExplains = [{ScanType:"ixscan", IndexName:"$id", ReturnNum:1}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
   
   println("check result after update index stat and analyze mode set 5 success!");
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the end" );
  
 }
 main()
 
 /************************************
*@Description: 指定_id插入不同记录,数据页超过10页
*@author:      zhaoyu
*@createDate:  2017.11.8
**************************************/
function insertDatas( dbcl, insertNum )
{  
   try
   {
      //插入不同记录
      var doc = [];
      for(var i = 0; i < insertNum; i++)
      {
         doc.push({_id:i,a:i,a0:i,a1:i,a2:i,a3:i,a4:i,a5:i,a6:i,a7:i,a8:i,a9:i,
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
      throw buildException("insertDatas()", e, "insert", "insert success", e);
   }
}