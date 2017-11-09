/************************************
*@Description: 指定唯一索引收集统计信息
*@author:      zhaoyu
*@createdate:  2017.11.9
*@testlinkCase:seqDB-11615
**************************************/
function main()
{
   var clName = COMMCLNAME + "_11615";
   var insertNum = 4000;
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the beginning" ) ;
   
   //创建cl
   var dbcl = commCreateCL( db, COMMCSNAME, clName);
   
   //创建索引
   commCreateIndex( dbcl, "a", {a:1}, true, true);
   
   //插入记录
	insertDatas( dbcl, insertNum );
	
	//检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", false, false );
   
   //检查主备节点访问计划
   var findConf = {a:1000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:1}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, COMMCSNAME, clName, findConf, null, null, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, COMMCSNAME, clName, findConf, null, null, expExplains )
	
	println("check result before analyze success!");

   //执行统计
   analyze( db, {Collection: COMMCSNAME + "." + clName, Index: "a"} );
   
   //检查统计信息
   checkStat( db, COMMCSNAME, clName, "a", true, true );
   
   //检查主备节点访问计划
   var findConf = {a:1000};
   var expExplains = [{ScanType:"ixscan", IndexName:"a", ReturnNum:1}];
   
   db.setSessionAttr( { PreferedInstance: "m" } );
   checkExplain( db, COMMCSNAME, clName, findConf, null, null, expExplains )
   
   db.setSessionAttr( { PreferedInstance: "s" } );
   checkExplain( db, COMMCSNAME, clName, findConf, null, null, expExplains )
   
   println("check result after analyze success!");
   
   //清理环境
   commDropCL( db, COMMCSNAME, clName, true, true,"drop CL in the end" );
  
 }
 main()
 
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
   }
   catch(e)
   {
      throw buildException("insertDatas()", e, "insert", "insert success", e);
   }
}