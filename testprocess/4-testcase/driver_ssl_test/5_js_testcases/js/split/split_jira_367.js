/******************************************************************************
*@Description : 1.SEQUOIADBMAINSTREAM-367:
*               split hang when specify the first partition as less than zero.
*               db.foo.bar.split("g3", "g1", {Partition:-1}, {Partition:500})
*
*@Modify list :
*               2014-10-15   xiaojun Hu Init
******************************************************************************/

function main( db )
{
   var replSize = new RSize( COMMCSNAME ).ReplSize( db ) ;
   var srcRg = "" ;
   var destRg = "" ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "clear collection in the beginning" ) ;
   // get group where collection space located in[ one cs and more group]
   var csRg = commGetCSGroups( db, COMMCSNAME ) ;
   var srcRg = csRg[0] ;
   // inspect have 2 data group
   var groups = commGetGroups( db ) ;
   if( groups.length < 2 )
   {
      println( db.listReplicaGroups() ) ;
      throw "don't have enough data group" ;
   }

   var cl = commCreateCLByOption( db, COMMCSNAME, COMMCLNAME, { ShardingKey:{"No":-1},
                                  ShardingType:"hash", Partition:1024,
                                  ReplSize:replSize },
                                  true, false, "create collection in split" ) ;
   // hash split, first partition value less than zero [TestPoint]
   try
   {
      cl.split( srcRg, destRg, {Partition: -10}, {Partition:600} )
      throw "<should not run split success>" ;
   }
   catch( e )
   {
      if( -6 != e )
      {
         println( "failed to run split , rc = " + e ) ;
         throw e ;
      }
   }

   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "clear collection in the end" ) ;
}

// Run Main
try
{
   if( false == commIsStandalone( db ) )
      main( db ) ;
   else
      println( "WARNING, run mode is standalone" ) ;
}
catch( e )
{
   if( "don't have enough data group" != e )
   {
      throw e ;
   }
   else
      println( "don't have enough data group" ) ;
}
