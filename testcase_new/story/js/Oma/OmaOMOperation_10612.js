/******************************************************************************
*@Description : test js object oma function: createOM removeOM
*               TestLink: 10612 Oma创建、删除sdbom服务进程
*                         10613 Oma创建sdbom服务进程，服务进程已存在
*@author      : Liang XueWang
******************************************************************************/

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
         throw "create om when om exist should be failed" ;
      }
      catch( e )
      {
         if( e != -145 )
         {
            throw buildException( "testOMOperation", e, 
                                  "createOm when om exist " + this, -145, e ) ;
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
         throw buildException( "testOMOperation", e, 
                               "createOm when om not exist " + this, 0, e ) ;
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
      // 测试OM的正常操作
      ots[i].testOMOperation( svcnames[i], OmExists[i] ) ;
   }
}

main()