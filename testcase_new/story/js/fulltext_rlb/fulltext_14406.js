/************************************
*@Description: 异常启动DB主节点不影响全文索引功能，且重启后新主为原主节点
*@author:      liuxiaoxuan
*@createdate:  2019.07.03
*@testlinkCase: seqDB-14406
**************************************/

function main()
{
   if( commIsStandalone( db ) )  {   return ;   }  

   var clName = COMMCLNAME + "_ES_14406";
   commDropCL( db, COMMCSNAME, clName, true, true );

   var dbcl = commCreateCL( db, COMMCSNAME, clName );
   
   // 插入数据，并检查集合所在数据组的lsn一致
   var objs = new Array();
   for( var i = 0; i < 20000; i++ )
   {
      objs.push( {a: "test_14406 " + i, b :  i } );
   }
   dbcl.insert( objs );
   checkConsistency( COMMCSNAME, clName );
   
   // 异常停止数据主节点
   var groups = commGetCLGroups( db, COMMCSNAME + "." + clName );
   var preMaster = db.getRG( groups[0] ).getMaster();
   var preMasterNodeName = preMaster.getHostName() + ":" + preMaster.getServiceName();
   try
   {
       // 加大原主节点的权重，使的后面重新选举尽可能选回自己
       db.updateConf( { "weight" : 100 }, { "NodeName" : preMasterNodeName } );
       var remote = new Remote( preMaster.getHostName(), CMSVCNAME ) ;
       var cmd = remote.getCmd() ;
       cmd.run( "ps -ef | grep sequoiadb | grep -v grep | grep " + preMaster.getServiceName() + " | awk '{print $2}' | xargs kill -9" );
       
       // 等待2min，检查数据组所有节点LSN是否一致
       checkGroupBusiness( 120, COMMCSNAME, clName );
   
       // 重新发起选主
       var doTimes = 1;
       for( ; doTimes <= 50; doTimes++ )
       {
           db.getRG( groups[0] ).reelect();
           // 等待选主
           isMasterNodeExist( groups[0] );
           var curMaster = db.getRG( groups[0] ).getMaster();
           var curMasterNodeName = curMaster.getHostName() + ":" + curMaster.getServiceName();
           println( "reelect times: " + doTimes + "\ncurMasterNodeName: " + curMasterNodeName + "\npreMasterNodeName: " + preMasterNodeName );
           // 当新主和原主为同一个节点，则退出
           if( preMasterNodeName == curMasterNodeName ) 
           {
               break;
           }
           sleep( 1000 );
       }
   
       // 选举后没有切回原主，则抛异常
       if ( doTimes > 50 )
       {
           throw buildException( "changePrimary", null, "reelect and change primary", preMasterNodeName, curMasterNodeName );
       }

       // 创建全文索引，检查数据同步
       var textIndexName = "textIndex_14406";   
       dbcl.createIndex( textIndexName, {"a" : "text"} );
       checkFullSyncToES( COMMCSNAME, clName, textIndexName, 20000 );
       checkConsistency( COMMCSNAME, clName );
   
       // 全文检索
       var findConf = {"$not": [{"b": {"$gte" : 10000}}, {"":{"$Text":{"query":{"match":{"a" : "test_14406"}}}}}]};
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
       // 等待环境恢复后再重置配置
       checkGroupBusiness( 120, COMMCSNAME, clName );
       db.updateConf( { "weight" : 10 }, { "NodeName" : preMasterNodeName } );
   } 
}
main();