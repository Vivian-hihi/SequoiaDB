/********************************************************************
*@Description : test Oma Initialization
*               TestLink: 10600  使用不存在的主机初始化Oma对象 
*                         10601  使用非cm端口初始化Oma对象
*@author      : Liang XueWang
********************************************************************/
function main()
{
   var illegalOma ;
   
   // 测试Oma使用不存在的主机初始化
   illegalOma = new OmaTest( "IllegalHost", CMSVCNAME, false, true ) ;
   illegalOma.testInit() ;
   
   // 测试Oma使用非cm端口初始化
   illegalOma = new OmaTest( COORDHOSTNAME, COORDSVCNAME, true, false ) ;
   illegalOma.testInit() ; 
}

main()