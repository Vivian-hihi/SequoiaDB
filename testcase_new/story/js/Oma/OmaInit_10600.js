/********************************************************************
*@Description : test Oma Initialization
*               TestLink: 10600 10601
*@author      : Liang XueWang
********************************************************************/

// 测试使用非cm端口初始化oma后，oma创建节点失败
OmaTest.prototype.testCreateCoordWithIllegalOma = function( svcname )
{
   this.testInit() ;
   try
   {
      var dbpath = RSRVNODEDIR + "coord/" + svcname ;
      this.oma.createCoord( svcname, dbpath ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( ( e == -15 ) || ( this.isStandalone && e == -159 ) )
         ;
      else
      {
         throw buildException( "testCreateCoordWithIllegalOma", e ) ;
      }   
   }
   this.oma.close() ;
}

function main()
{
   var ot ;
   try
   {
      // 测试Oma使用不存在的主机初始化
      ot = new OmaTest( "IllegalHost", CMSVCNAME, false ) ;
      ot.testInit() ;
      
      // 测试Oma使用非cm端口初始化（初始化时不报错，创建节点时报错）
      ot = new OmaTest( COORDHOSTNAME, COORDSVCNAME, true, false ) ;
      ot.testInit() ;
      
      // 获取一个未被占用的端口
      var svcname = toolGetIdleSvcName( COORDHOSTNAME, CMSVCNAME ) ;
      if( svcname === undefined )
      {
         println( "No idle svcname between RSRVPORTBEGIN and RSRVPORTEND" ) ;
         return ;
      }
      
      // 测试创建节点时报错
      ot.testCreateCoordWithIllegalOma( svcname ) ;
   }
   catch( e )
   {
      ot.toString() ;
      throw e ;
   } 
}

main()