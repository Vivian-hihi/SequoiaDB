/************************************
*@Description: 正常启动DB主节点不影响全文索引功能，且重启后新主为新主节点
*@author:      liuxiaoxuan
*@createdate:  2019.08.21
*@testlinkCase: seqDB-14405
**************************************/

function main()
{
   if( commIsStandalone( db ) )  {   return ;   }  

   var clName = COMMCLNAME + "_ES_14405B";
   commDropCL( db, COMMCSNAME, clName, true, true );

   var dbcl = commCreateCL( db, COMMCSNAME, clName );
   
   // 创建全文索引，插入数据
   var textIndexName = "textIndex_14405B";   
   dbcl.createIndex( textIndexName, {"a" : "text"} );
   var objs = new Array();
   for( var i = 0; i < 20000; i++ )
   {
      objs.push( {a: "test_14405B " + i, b :  i } );
   }
   dbcl.insert( objs );
   
   // 正常停止数据主节点
   var groups = commGetCLGroups( db, COMMCSNAME + "." + clName );
   var preMaster = db.getRG( groups[0] ).getMaster();
   var preSlave = db.getRG( groups[0] ).getSlave();
   var preSlaveNodeName = preSlave.getHostName() + ":" + preSlave.getServiceName(); 
   try
   {
       // 加大备节点的权重，使的后面重新选举尽可能选到该节点
       db.updateConf( { "weight" : 100 }, { "NodeName" : preSlaveNodeName } );
       preMaster.stop(); 
       preMaster.start();       
       
       // 等待2min，检查数据组所有节点LSN是否一致
       checkGroupBusiness( 120, COMMCSNAME, clName );
   
       // 重新发起选主
       var doTimes = 1;
       for( ; doTimes <= 50; doTimes++ )
       {
           try
           {
               db.getRG( groups[0] ).reelect();
               // 等待选主
               isMasterNodeExist( groups[0] );
               var curMaster = db.getRG( groups[0] ).getMaster();
               var curMasterNodeName = curMaster.getHostName() + ":" + curMaster.getServiceName();
               println( "reelect times: " + doTimes + "\ncurMasterNodeName: " + curMasterNodeName + "\npreSlaveNodeName: " + preSlaveNodeName );
               // 当新主为原备节点，则退出
               if( preSlaveNodeName == curMasterNodeName ) 
               {
                   break;
               }
               sleep( 1000 );
           } 
           catch ( e ) 
           {
               if ( -13 != e )
               {
                   throw buildException( "reelect", null, "reelect", "-13", e );
               }
           } 
       }
   
       // 选举后没有切回原主，则抛异常
       if ( doTimes > 50 )
       {
           throw buildException( "changePrimary", null, "reelect and change primary", preSlaveNodeName, curMasterNodeName );
       }
       
       // 执行增删改
       dbcl.insert( [{ a : 'test_14405B 20001', b : 20001}, { a : 'test_14405B 20002', b : 20002}, { a : 'test_14405B 20003', b : 20003}] );
       dbcl.update( { $set : { a : "test_14405B update" } } , {a : "test_14405B 10001"} );
       dbcl.remove( {a : "test_14405B 10002"} );

       // 检查数据同步
       checkFullSyncToES( COMMCSNAME, clName, textIndexName, dbcl.count() );
       checkConsistency( COMMCSNAME, clName );
   
       // 全文检索
       var findConf = {"$not": [{"b": {"$gte" : 10000}}, {"":{"$Text":{"query":{"match":{"a" : "test_14405B"}}}}}]};
       var actResult = dbOpr.findFromCL( dbcl, findConf, {'a' : ''} );
       var expResult = dbOpr.findFromCL( dbcl, {"b": {"$lt" : 10000}}, {'a' : ''} );
       actResult.sort( compare("a") );
       expResult.sort( compare("a") );
       checkResult( expResult, actResult );
       println( "---check result success---" );
   
       var esIndexNames = dbOpr.getESIndexNames( COMMCSNAME, clName, textIndexName );
       commDropCL( db, COMMCSNAME, clName, true, true ); 
       //SEQUOIADBMAINSTREAM-3983
       checkIndexNotExistInES( esIndexNames );      
   }
   finally
   { 
       // 重置配置
       db.updateConf( { "weight" : 10 }, { "NodeName" : preSlaveNodeName } );
       preMaster.start();
   }

}
main();