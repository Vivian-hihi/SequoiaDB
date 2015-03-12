/******************************************************************************
@Description : 1. Test split and insert thousands data.[query/update/remove]
@Modify list :
               2014-6-25  xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   var replSize = new RSize( COMMCSNAME ).ReplSize( db ) ;
   // Inspect and Get groups
   var group = new Array() ;
   group = getGroup( db ) ;
   if( group.length < 2 )
   {
      println( "Don't have enough group, count = " + group.length ) ;
      throw "NotEnoughGroup" ;
   }
   // Create CS and CL
   commCreateCLByOption( db, COMMCSNAME, COMMCLNAME, { ShardingKey:{"No":-1},
                         ShardingType:"hash", Partition:1024 ,
                         ReplSize:replSize },
                         true, false, "create collection in domain" ) ;
   // Get Group the COMMCSNAME located in and Split the CS to other group
   // [Testing Point ]
   getTwoGroupSplit( db, COMMCSNAME, COMMCLNAME, {Partition : 512},
                     {Partition : 1024} ) ;
   // Insert Data
   insertData( db, COMMCSNAME, COMMCLNAME, 10000 ) ;
   println( "Success to insert data, count = 10000" ) ;
   // Query Data
   queryData( db, COMMCSNAME, COMMCLNAME ) ;
   // Update Data
   updateData( db, COMMCSNAME, COMMCLNAME ) ;
   // Remove Data
   removeData( db, COMMCSNAME, COMMCLNAME ) ;
}

try
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "clear collection space in the beginning" ) ;
   // Inspect the run mode is standalone or not
   if( true == commIsStandalone( db ) )
      throw "ModeStandAlone" ;
   main( db ) ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "clear collection space in the end, correct" ) ;
   db.close() ;
}
catch( e )
{
   if( "NotEnoughGroup" == e )
      println( "Don't have enough group" ) ;
   else if( "ModeStandAlone" == e )
      println( "This is standalone mode" ) ;
   else
   {
      commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
                  "clear collection space in the end, correct" ) ;
      db.close() ;
      throw e ;
   }
}
