/******************************************************************************
*@Description : test function createCoord,removeCoord,createData,removeData
*               startNode,stopNode,close Abnormal
*               TestLink: 10603 10604 10607 10608 10609 10610 10611 10614 10615
*                         10616
*@author      : Liang XueWang               
******************************************************************************/

// 测试创建已存在的节点
OmaTest.prototype.testCreateExistCoord = function()
{
   this.testInit() ;
   try
   {
      var svcname = COORDSVCNAME ;
      var dbpath = RSRVNODEDIR + "coord/" + svcname ;
      this.oma.createCoord( svcname, dbpath ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( e != -145 )
      {
         throw buildException( "testCreateExistCoord", e, "create exist coord", -145, e ) ;
      }
   }
   this.oma.close() ;
}

// 测试删除不存在的节点
OmaTest.prototype.testRemoveNotExistCoord = function( svcname )
{
   this.testInit() ;
   try
   {
      this.oma.removeCoord( svcname ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( e != -146 )
      {
         throw buildException( "testRemoveNotExistCoord", e, "remove not exist coord", -146, e ) ;
      }
   }
   this.oma.close() ;
}

// 测试删除节点时使用错误端口
OmaTest.prototype.testRemoveCoordWithWrongSvc = function()
{
   this.testInit() ;
   try
   {
      this.oma.removeCoord( CMSVCNAME ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( e != -146 )
      {
         throw buildException( "testRemoveCoordWithWrongSvc", e, 
                               "remove coord with cmsvcname", -146, e ) ;
      }
   }
   this.oma.close() ;
}

// 测试删除节点时使用错误配置项
OmaTest.prototype.testRemoveCoordWithWrongConf = function()
{
   this.testInit() ;
   try
   {
      this.oma.removeCoord( COORDSVCNAME, { clustername: "!@#$%^&*" } ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( ( e == -146 ) || ( this.isStandalone && e == -3 ) )
         ;
      else
      {
         throw buildException( "testRemoveCoordWithWrongConf", e, 
                               "remove coord with wrong config", -146, e ) ;
      }
   }
   this.oma.close() ;
}

// 测试启动不存在的节点
OmaTest.prototype.testStartNotExistNode = function( svcname )
{
   this.testInit() ;
   try
   {
      this.oma.startNode( svcname ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( e != -146 )
      {
         throw buildException( "testStartNotExistNode", e, "start not exist node", -146, e ) ;
      }
   }
   this.oma.close() ;
}

// 测试停止不存在的节点
OmaTest.prototype.testStopNotExistNode = function( svcname )
{
   this.testInit() ;
   try
   {
      this.oma.stopNode( svcname ) ;
   }
   catch( e )
   {
      throw buildException( "testStopNotExistNode", e, "stop not exist node", 0, e ) ;
   }
   this.oma.close() ;
}

// 测试创建节点时使用错误配置项
OmaTest.prototype.testCreateCoordWithWrongConf = function( svcname )
{
   this.testInit() ;
   try
   {
      var dbpath = RSRVNODEDIR + "coord/" + svcname ;
      // 不正确的配置项 role: om 配置文件自动修改为 role: coord
      this.oma.createCoord( svcname, dbpath, { role: "om" } ) ;
      this.oma.startNode( svcname ) ;
      this.oma.stopNode( svcname ) ;
      this.oma.removeCoord( svcname ) ;
      // 不存在的配置项 abc: 123 写入配置文件
      this.oma.createCoord( svcname, dbpath, { abc: "123" } ) ;
      this.oma.startNode( svcname ) ;
      this.oma.stopNode( svcname ) ;
      this.oma.removeCoord( svcname ) ;
   }
   catch( e )
   {
      throw buildException( "testCreateCoordWithWrongConf", e, 
                            "create coord with wrong config", 0, e ) ;
   }
   this.oma.close() ;
}

// 测试创建节点时节点目录无权限
OmaTest.prototype.testCreateCoordWithNoPermit = function( svcname )
{
   this.testInit() ;
   try
   {
      this.oma.createCoord( svcname, "/root/"+svcname ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( e != -3 )
      {
         throw buildException( "testCreateCoordWithNoPermit", e, 
                               "create coord with no permit", -3, e ) ;
      }
   }
   this.oma.close() ;
}

// 测试oma关闭后执行操作
OmaTest.prototype.testOmaClose = function()
{
   this.testInit() ;
   this.oma.close() ;
   try
   {
      this.oma.stopNode( COORDSVCNAME ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( e != -6 )
      {
         throw buildException( "testOmaClose", e, "stop coord node after close", -6, e ) ;
      }
   }
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
   
   var ot1 = new OmaTest( localhost, CMSVCNAME ) ;
   var ot2 = new OmaTest( remotehost, CMSVCNAME ) ;
   var ots = [ ot1, ot2 ] ;
   var svcnames = [ svcname1, svcname2 ] ;
   
   for( i = 0;i < ots.length;i++ )
   {
      try
      {
         // 测试创建已存在的节点
         ots[i].testCreateExistCoord() ;
         
         // 测试删除不存在的节点
         ots[i].testRemoveNotExistCoord( svcnames[i] ) ;
         
         // 测试删除节点时端口号不匹配
         ots[i].testRemoveCoordWithWrongSvc() ;
         
         // 测试删除节点时配置项非法
         ots[i].testRemoveCoordWithWrongConf() ;
         
         // 测试启动不存在的节点
         ots[i].testStartNotExistNode( svcnames[i] ) ;
         
         // 测试停止不存在的节点
         ots[i].testStopNotExistNode( svcnames[i] ) ;
         
         // 测试创建节点时配置项非法
         ots[i].testCreateCoordWithWrongConf( svcnames[i] ) ;
         
         // 测试创建节点时路径无权限
         ots[i].testCreateCoordWithNoPermit( svcnames[i] ) ;
         
         // 测试oma关闭后执行操作
         ots[i].testOmaClose() ;
      }
      catch( e )
      {
         ots[i].toString() ;
         throw e ;
      }
   }
}

main()