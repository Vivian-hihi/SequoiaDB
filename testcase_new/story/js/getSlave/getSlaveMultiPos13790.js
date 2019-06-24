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
   testTwoNodeMasterPos() ;
   testThreeNode() ;
}

// only one node in rg, test getSlave with 1-7
function testOneNode()
{
   var rg = db.createRG( rgName ) ;
   createNodes( rg, 1 ) ;
   
   var master = getMaster( rg ); 
   var slave = rg.getSlave( 1, 2, 3, 4, 5, 6, 7 ) ;
   if( slave.toString() !== master.toString() )
   {
      throw buildException( "testOneNode", null, "check getSlave", master, slave ) ;
   }
}

// two nodes in rg, test getSlave with 1-7
function testTwoNode()
{
   var rg = db.getRG( rgName ) ;
   createNodes( rg, 1 ) ;
   
   var master = getMaster( rg );
   var slave = rg.getSlave( 1, 2, 3, 4, 5, 6, 7 ) ;
   if( slave.toString() === master.toString() )
   {
      throw buildException( "testTwoNode", null, "check getSlave", slave, master ) ;
   }
}

// two nodes in rg, getSlave with all master pos
function testTwoNodeMasterPos()
{
   var rg = db.getRG( rgName ) ;
   var master = getMaster( rg );
   
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
}

// three nodes in rg( master change ), getSlave with 1-7
function testThreeNode()
{
   var rg = db.getRG( rgName ) ;
   createNodes( rg, 1 ) ;
   
   var master = getMaster( rg );
   master.stop() ;
   var newMaster = getMaster( rg );
   
   var slave = rg.getSlave( 1, 2, 3, 4, 5, 6, 7 ) ;
   if( slave.toString() === newMaster.toString() )
   {
      throw buildException( "testThreeNode", null, "check the returned slave node", newMaster.toString(), slave.toString() ) ;
   }
   
   db.removeRG( rgName ) ;
}
