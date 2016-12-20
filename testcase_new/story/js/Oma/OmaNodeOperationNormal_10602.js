/******************************************************************************
*@Description : test Oma function createCoord removeCoord createData
*               removeData startNode stopNode createOM removeOM Normal
*               TestLink: 10602 10612 10613
*@author      : Liang XueWang
******************************************************************************/

// 测试正常创建启动停止删除协调节点
OmaTest.prototype.testCoordNodeOperationNormal = function( svcname )
{
   this.testInit() ;
   try
   {
      var dbpath = RSRVNODEDIR + "coord/" + svcname ;
      this.oma.createCoord( svcname, dbpath ) ;
      this.oma.startNode( svcname ) ;
      this.oma.stopNode( svcname ) ;
      this.oma.removeCoord( svcname ) ; 
   }
   catch( e )
   {
      throw buildException( "testCoordNodeOperationNormal", e ) ;
   }
   this.oma.close() ;  
}

// 测试正常创建启动停止删除数据节点
OmaTest.prototype.testDataNodeOperationNormal = function( svcname )
{
   this.testInit() ;
   try
   {
      var dbpath = RSRVNODEDIR + "coord/" + svcname ;
      this.oma.createData( svcname, dbpath ) ;
      this.oma.startNode( svcname ) ;
      this.oma.stopNode( svcname ) ;
      this.oma.removeData( svcname ) ; 
   }
   catch( e )
   {
      throw buildException( "testDataNodeOperationNormal", e ) ;
   }
   this.oma.close() ;   
}

// 测试创建删除om
OmaTest.prototype.testOMOperation = function( svcname, isOmExist )
{
   this.testInit() ;
   var dbpath = RSRVNODEDIR + "sms/" + svcname ;
   if( isOmExist )
   {
      try
      {
         this.oma.createOM( svcname, dbpath ) ;
         throw 0 ;
      }
      catch( e )
      {
         if( e != -145 )
         {
            throw buildException( "testOMOperation", e, "createOm when om exist", -145, e ) ;
         }
      }
   }
   else
   {
      try
      {
         this.oma.createOM( svcname, dbpath ) ;
      }
      catch( e )
      {
         throw buildException( "testOMOperation", e, "createOm when om not exist", 0, e ) ;
      }
      this.oma.removeOM( svcname ) ;
   }
   this.oma.close() ;
}

function main()
{
   // 获取本地主机和远程主机
   var localhost = toolGetLocalhost() ;
   var remotehost = toolGetRemotehost() ;
   
   // 获取本地和远程空闲的端口号
   var svcname1 = toolGetIdleSvcName( localhost, CMSVCNAME ) ;
   if( svcname1 === undefined )
   {
      println( "No idle svcname between RSRVPORTBEGIN and RSRVPORTEND local" ) ;
      return ;
   }
   var svcname2 = toolGetIdleSvcName( remotehost, CMSVCNAME ) ;
   if( svcname2 === undefined )
   {
      println( "No idle svcname between RSRVPORTBEGIN and RSRVPORTEND remote" ) ;
      return ;
   }
   
   // 判断OM是否存在
   var OmExist1 = isOmExist( localhost, CMSVCNAME ) ;
   var OmExist2 = isOmExist( remotehost, CMSVCNAME ) ;
   
   var ot1 = new OmaTest( localhost, CMSVCNAME ) ;
   var ot2 = new OmaTest( remotehost, CMSVCNAME ) ;
   
   var ots = [ ot1, ot2 ] ;
   var svcnames = [ svcname1, svcname2 ] ;
   var OmExists = [ OmExist1, OmExist2 ] ;
   
   for( var i = 0;i < ots.length;i++ )
   {
      try
      {
         // 测试协调节点的正常操作
         ots[i].testCoordNodeOperationNormal( svcnames[i] ) ;
         
         // 测试数据节点的正常操作
         ots[i].testDataNodeOperationNormal( svcnames[i] ) ;
         
         // 测试OM的正常操作
         ots[i].testOMOperation( svcnames[i], OmExists[i] ) ;
      }
      catch( e )
      {
         ots[i].toString() ;
         throw e ;
      }
   }
}

main()