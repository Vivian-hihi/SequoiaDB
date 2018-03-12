/*******************************************************************
* @Description : stop and start multi groups
*                seqDB-14663:停止、启动多个数据组                
* @author      : Liang XueWang
*                2018-03-12
*******************************************************************/
main( db ) ;

function main( db )
{
   if( commIsStandalone( db ) )
   {
      println( "Run mode is standalone" ) ;
      return ;
   }
   var groups = getDataGroups( db ) ;
   if( groups.length <= 1 )
   {
      println( "Groups num too few" ) ;
      return ;
   }
   var group1 = groups[0] ;
   var group2 = groups[1] ;
   
   // stop group1 group2
   db.stopRG( group1, group2 ) ;
   
   // check stop
   checkGroupStatus( db, group1, "stop" ) ;
   checkGroupStatus( db, group2, "stop" ) ;
   
   // start group1 group2
   db.startRG( group1, group2 ) ;
   
   // check start
   checkGroupStatus( db, group1, "start" ) ;
   checkGroupStatus( db, group2, "start" ) ;
   
   // wait primary choosed
   waitPrimary( db, group1 ) ;
   waitPrimary( db, group2 ) ;
}

function checkGroupStatus( db, groupname, status )
{
   var nodes = parseGroupNodes( db, groupname ) ;
   for( var i = 0;i < nodes.length;i++ )
   {
      try
      {
         var dataDb = new Sdb( nodes[i] ) ;
         if( status === "stop" ) throw 0 ;
      }
      catch( e )
      {
         if( status === "stop" && e === -15 ) 
            ;  // when group stopped, connect throw -15, do nothing
         else
         {
            var expectErr = ( status === "stop" ) ? -15 : 0 ;
            throw buildException( "checkGroupStatus", e, 
                  "check node " + nodes[i] + " status", expectErr, e ) ;
         }
      }
   }
}

function waitPrimary( db, groupname )
{
   do {
      try
      {
         var rg = db.getRG( groupname ) ;
         var node = rg.getMaster() ;
         var dataDb = node.connect() ;
         var isPrimary = dataDb.snapshot( SDB_SNAP_DATABASE ).next().toObj()["IsPrimary"] ;
         dataDb.close() ;
      }
      catch( e )
      {
         println( "waitPrimary throw " + e ) ;
         sleep( 1000 ) ;
         continue ;
      }
   } while( !isPrimary ) ;
}