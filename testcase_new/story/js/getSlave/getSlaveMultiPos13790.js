/******************************************************************************
 * @Description : test getSlave operation
 *                seqDB-13790:指定多个位置获取备节点
 * @auhor       : Liang XueWang
 ******************************************************************************/
var rgName = "testGetSlaveRg13790" ;

main() ;

function main()
{
   if( commIsStandalone( db ) )
   {
      println( "Run mode is standalone" ) ;
      return ;
   }
   
   testOneNode() ;
   testTwoNode() ;
   //问题单：SEQUOIADBMAINSTREAM-3495
   //testTwoNodeMasterPos() ;
   testThreeNode() ;
}

// only one node in rg, test getSlave with 1-7
function testOneNode()
{
   var rg = db.createRG( rgName ) ;
   createNodes( rg, 1 ) ;
   rg.start() ;
   
   var master ;
   while( true )
   {
      try
      {
         master = rg.getMaster() ;
         break ;
      }
      catch( e )
      {
         if( e == -71 )
         {
            continue ;
         }
         else
         {
            throw buildException( "testOneNode", e, "get master", "0 -71", e ) ;
         }
      }
   }
   
   var slave = rg.getSlave( 1, 2, 3, 4, 5, 6, 7 ) ;
   if( slave.toString() !== master.toString() )
   {
      throw buildException( "testOneNode", null, "check getSlave", master, slave ) ;
   }
   
   db.removeRG( rgName ) ;
}

// two nodes in rg, test getSlave with 1-7
function testTwoNode()
{
   var rg = db.createRG( rgName ) ;
   createNodes( rg, 2 ) ;
   rg.start() ;
   
   var master ;
   while( true )
   {
      try
      {
         master = rg.getMaster() ;
         break ;
      }
      catch( e )
      {
         if( e == -71 )
         {
            continue ;
         }
         else
         {
            throw buildException( "testTwoNode", e, "get master", "0 -71", e ) ;
         }
      }
   }
   
   var slave = rg.getSlave( 1, 2, 3, 4, 5, 6, 7 ) ;
   if( slave.toString() === master.toString() )
   {
      throw buildException( "testTwoNode", null, "check getSlave", slave, master ) ;
   }
   
   db.removeRG( rgName ) ;
}

// two nodes in rg, getSlave with all master pos
function testTwoNodeMasterPos()
{
   var rg = db.createRG( rgName ) ;
   createNodes( rg, 2 ) ;
   rg.start() ;
   
   var master ;
   while( true )
   {
      try
      {
         master = rg.getMaster() ;
         break ;
      }
      catch( e )
      {
         if( e == -71 )
         {
            continue ;
         }
         else
         {
            throw buildException( "testTwoNodeNoMaster", e, "get master", "0 -71", e ) ;
         }
      }
   }
   
   var nodes = getGroupNodes( db, rgName ) ;
   var masterIdx = nodes.indexOf( master.toString() ) ;
   if( masterIdx === -1 )
   {
      throw buildException( "testTwoNodeMasterPos", null, "get master pos", "!= -1", masterPos ) ;
   }
   var masterPos = masterIdx + 1 ;

   var slave = rg.getSlave( masterPos, masterPos, masterPos ) ;
   if( slave.toString() !== master.toString() )
   {
      throw buildException( "testTwoNodeMasterPos", null, "check getSlave", master, slave ) ;
   }
   
   db.removeRG( rgName ) ;
}

// three nodes in rg( master change ), getSlave with 1-7
function testThreeNode()
{
   var rg = db.createRG( rgName ) ;
   createNodes( rg, 3 ) ;
   rg.start() ;
   
   var master ;
   while( true )
   {
      try
      {
         master = rg.getMaster() ;
         break ;
      }
      catch( e )
      {
         if( e == -71 )
         {
            continue ;
         }
         else
         {
            throw buildException( "testThreeNode", e, "get master", "0 -71", e ) ;
         }
      }
   }
   master.stop() ;
   var newMaster ;
   while( true )
   {
      try
      {
         newMaster = rg.getMaster() ;
         if( newMaster.toString() === master.toString() )
            continue ;
         break ;
      }
      catch( e )
      {
         throw buildException( "testThreeNode", e, "getMaster", 0, e ) ; 
      }
   }

   var totalCnt = 50 ;
   var masterCnt = 0 ;
   var newMasterCnt = 0 ;
   for( var i = 0;i < totalCnt;i++ )
   {
      var slave = rg.getSlave( 1, 2, 3, 4, 5, 6, 7 ) ;
      if( slave.toString() === master.toString() )
         masterCnt++ ;
      else if( slave.toString() === newMaster.toString() )
         newMasterCnt++ ;
   }
   if( masterCnt === 0 || newMasterCnt !== 0 )
   {
      throw buildException( "testThreeNode", null, "check cnt", "!0 0", masterCnt + " " + newMasterCnt ) ;
   }
   
   db.removeRG( rgName ) ;
}
