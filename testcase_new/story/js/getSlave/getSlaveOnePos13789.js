/******************************************************************************
*@Description : test getSlave operation
*               seqDB-13789:指定一个位置获取备节点
*@auhor       : Liang XueWang
******************************************************************************/
var rgName = "testGetSlaveRg13789" ;

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
   testTwoNodeNoMaster() ;
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
   
   for( var i = 1;i <= 7;i++ )
   {
      var slave = rg.getSlave( i ) ;
      if( slave.toString() !== master.toString() )
      {
         throw buildException( "testOneNode", null, "check getSlave", master, slave ) ;
      }
   }
   
   db.removeRG( rgName ) ;
}

// two nodes in rg, test getSlave
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
   
   var nodes = getGroupNodes( db, rgName ) ;
   for( var i = 1;i <= 7;i++ )
   {
      var slave = rg.getSlave( i ) ;
      var idx = ( i - 1 ) % 2 ;
      if( slave.toString() !== nodes[idx] )
      {
         throw buildException( "testTwoNode", null, "check getSlave", nodes[idx], slave ) ;
      }
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
   
   var nodes = getGroupNodes( db, rgName ) ;
   for( var i = 1;i <= 7;i++ )
   {
      var slave = rg.getSlave( i ) ;
      var idx = ( i - 1 ) % 2 ;
      if( slave.toString() !== nodes[idx] )
      {
         throw buildException( "testTwoNodeNoMaster", null, "check getSlave", nodes[idx], slave ) ;
      }
   }
   
   db.removeRG( rgName ) ;
}