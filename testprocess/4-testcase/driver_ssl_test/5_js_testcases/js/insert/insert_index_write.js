/*******************************************************************************
*@Description : First create index for collection and then insert data in it
*@Modify List :
*               2014-9-26   xiaojun Hu   Changed
*******************************************************************************/
function main( db )
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "failed to drop collection in the beginning" ) ;
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, -1, true, true, false,
                          "failed to create collection in the beginning" ) ;
   // create index
   cl.createIndex( "noIndex", {no:-1}, true, true ) ;
   println( "create index success" ) ;
   commCheckIndex( cl, "noIndex", true, 10 ) ;   // timeout = 10
   // insert data
   insertAndCheck( cl, 100, true, true, "failed to insert 100 records to sdb " ) ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "failed to drop collection in the end" ) ;
}

try
{
   main( db ) ;
}
catch( e )
{
   throw e ;
}

