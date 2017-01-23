/********************************************************************
*@Description : test Oma Initialization
*               TestLink: 10600  使用不存在的主机初始化Oma对象 
*                         10601  使用非cm端口初始化Oma对象
*@author      : Liang XueWang
********************************************************************/

// 测试使用非cm端口初始化oma后，oma创建节点失败
OmaTest.prototype.testCreateDataWithIllegalOma = function( svcname )
{
   this.testInit() ;
   
   if( this.isStandalone )
   {
      // println( "Run mode is standalone." ) ;
      return ;
   }
   
   try
   {
      var dbpath = RSRVNODEDIR + svcname ;
      this.oma.createData( svcname, dbpath ) ;
      throw "create data with illegal oma should be failed" ;
   }
   catch( e )
   {
      if(  e !== -15  )
      {
         println( "create data " + svcname + " dbpath " + dbpath ) ;
         throw buildException( "testCreateDataWithIllegalOma", e,
               "test create data with illegal oma " + this, "-15", e ) ;
      }   
   }
   
   this.oma.close() ;
}

function main()
{
   var illegalOma ;
   
   // 测试Oma使用不存在的主机初始化
   illegalOma = new OmaTest( "IllegalHost", CMSVCNAME, false ) ;
   illegalOma.testInit() ;
   
   // 测试Oma使用非cm端口初始化（初始化时不报错，创建节点时报错）
   illegalOma = new OmaTest( COORDHOSTNAME, COORDSVCNAME, true, false ) ;
   illegalOma.testInit() ;
   
   // 获取一个未被占用的端口
   var svcname = toolGetIdleSvcName( COORDHOSTNAME, CMSVCNAME ) ;
   if( svcname === undefined )
   {
      println( "No idle svcname between RSRVPORTBEGIN and RSRVPORTEND" ) ;
      return ;
   }
   
   // 测试创建节点时报错
   illegalOma.testCreateDataWithIllegalOma( svcname ) ; 
}

main()