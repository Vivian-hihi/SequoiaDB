/******************************************************************************
*@Description : test oma svcname boundary number
*               TestLink: 10605 Oma创建节点，端口号为边界值   65535，1 
*                         10606 Oma创建节点，端口号超过边界值 65536，0
*@author      : Liang XueWang
******************************************************************************/

// 测试创建节点时端口号为边界值及边界值以内( 0, 65536 )
OmaTest.prototype.testSvcnameBoundary = function()
{
   this.testInit() ;
   var ErrorSvcname = [ 0, 65536 ] ;
   var CorrSvcname = [ 1, 65535 ] ;
   for( var i = 0;i < ErrorSvcname.length;i++ )
   {
      try
      {
         var svcname = ErrorSvcname[i] ;
         var dbpath = RSRVNODEDIR + "data/" + svcname ;
         this.oma.createData( svcname, dbpath ) ;
         throw "create data with " + svcname + "should be failed" ;
      }
      catch( e )
      {
         if( e != -6 )
         {
            throw buildException( "testSvcnameBoundary", -6, 
                                  "create data " + this, -6, e ) ;
         }
      }
   }
   for( var i = 0;i < CorrSvcname.length;i++ )
   {
      try
      {
         var svcname = CorrSvcname[i] ;
         var dbpath = RSRVNODEDIR + "data/" + svcname ;
         this.oma.createData( svcname, dbpath ) ;
         this.oma.removeData( svcname ) ;
      }
      catch( e )
      {
         throw buildException( "testSvcnameBoundary", 0, 
                               "create data " + this, 0, e ) ;
      }   
   }
   this.oma.close() ;
}

function main()
{
   // 获取本地主机和远程主机
   var localhost = toolGetLocalhost() ;
   var remotehost = toolGetRemotehost() ;
   
   var ot1 = new OmaTest( localhost, CMSVCNAME ) ;
   var ot2 = new OmaTest( remotehost, CMSVCNAME ) ;
   var ots = [ ot1, ot2 ] ;
   
   for( var i = 0;i < ots.length;i++ )
   {
      // 测试端口号取边界值及超出边界时创建节点（0,65536）（1,65535）
      // ots[i].testSvcnameBoundary() ;
   }
}

main()