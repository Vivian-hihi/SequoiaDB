/*******************************************************************************
*@Description : When we create index for querying, test query use selector:
*               $include/$default/$slice/$elemMatchOne/$elemMatch
*@Modify list :
*               2015-01-29  xiaojun Hu  Change
*******************************************************************************/

function main( db )
{
   try
   {
      var recordNum = 100 ;
      var idxName = "rgIdIndex" ;
      var idxDef = { "GroupID":-1, "PrimaryNode":1, "Version":1 } ;
      var addRecord = { "nest1":{"nest2":{"nest3":"when nest test, use $include"}}} ;
      var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, false,
                             "create colleciton in the begnning" ) ;
      // auto generate data and create Index
      commCreateIndex( cl, idxName, idxDef ) ;
      selAutoGenData( cl, recordNum, addRecord ) ;
      println( "success to create index: " + idxName +
               " and insert record: " + recordNum ) ;

      /*Test Point 1 $include */
      var condObj = {} ;
      var selObj = { "GroupID": {"$include":1},
                     "Group.Service.Name": {"$include":1},
                     "Group.HostName": {"$include":1},
                     "Version": {"$include":1} } ;
      var ret = selMainQuery( cl, condObj, selObj ) ;
      println( "==>success to test use: " + JSON.stringify( selObj ) ) ;
      var condObj = {} ;
      var selObj = { "GroupID": {"$include":0},
                     "Group.Service.Name": {"$include":0},
                     "Group.HostName": {"$include":0},
                     "PrimaryNode": {"$include":0} } ;
      var ret = selMainQuery( cl, condObj, selObj ) ;
      println( "==>success to test use: " + JSON.stringify( selObj ) ) ;
      println( "success to query when use $include selector" ) ;

   }
   catch( e )
   {
      throw e ;
   }
}

// Run Main
try
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "drop collection in the begining" ) ;
   main( db ) ;
   //commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
   //            "drop collection in the end") ;
}
catch( e )
{
   //commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
   //            "drop collection in the end") ;
   throw e ;
}
