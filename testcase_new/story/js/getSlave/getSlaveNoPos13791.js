/******************************************************************************
 * @Description : test getSlave operation
 *                seqDB-13791:使用getSlave()获取备节点 
 * @auhor       : Liang XueWang
 ******************************************************************************/
var rgName = "testGetSlaveRg13791" ;

main() ;

function main()
{
   if( commIsStandalone( db ) )
   {
      println( "Run mode is standalone" ) ;
      return ;
   }
   
   testOneNode() ;
   testTwoNodeNoMaster() ;
   testThreeNode() ;
}

// only one node in rg, test getSlave
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
   
   var slave = rg.getSlave() ;
   if( slave.toString() !== master.toString() )
   {
      throw buildException( "testOneNode", null, "check getSlave", master, slave ) ;
   }
   
   db.removeRG( rgName ) ;
}

// two nodes in rg( no master ), test getSlave
function testTwoNodeNoMaster()
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
   master.stop() ;
   var hasMaster = isMasterExist( db, rgName ) ;
   if( hasMaster !== false )
   {
      throw buildException( "testTwoNodeNoMaster", null, "check no master", false, hasMaster ) ;
   }
   
   var totalCnt = 50 ;
   for( var i = 0;i < totalCnt;i++ )
   {
      var slave = rg.getSlave() ;
      if( slave.toString() === master.toString() )
      {
         throw buildException( "testTwoNodeNoMaster", null, "check getSlave", slave, master ) ;
      }
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
   var nodes = getGroupNodes( db, rgName ) ;
   var masterIdx = nodes.indexOf( master.toString() ) ;

   var totalCnt = 50 ;
   var cnt = [ 0, 0, 0 ] ;
   for( var i = 0;i < totalCnt;i++ )
   {
      var slave = rg.getSlave() ;
      var idx = nodes.indexOf( slave.toString() ) ;
      cnt[idx]++ ;
   }
   println( "cnt: " + cnt[0] + " " + cnt[1] + " " + cnt[2] ) ;
   if( cnt[masterIdx] !== 0 )
   {
      throw buildException( "testThreeNode", null, "check master cnt", 0, cnt[masterIdx] ) ;
   }
   
   db.removeRG( rgName ) ;
}
