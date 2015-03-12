/******************************************************************************
@Description : Public function for testing split.
@Modify list :
               2014-6-17  xiaojun Hu  Init
******************************************************************************/

var hostName = COORDHOSTNAME ;
var coordPort = COORDSVCNAME ;
var cataPort = CATASVCNAME ;
var csName = COMMCSNAME ;
var clName = COMMCLNAME ;

var db = new SecureSdb( hostName, coordPort ) ;

// Get data group and split
function splitGroup( db, csName, inserNum )
{
   try
   {
      var groups = new Array() ;
      groups = commGetGroups( db, "GroupName", "", true, true ) ;
      var csGroup = new Array() ;
      csGroup = commGetCSGroups( db, csName ) ;
      var dNum = groups.length - 1 ;
      var cs = db.getCS( csName ) ;
      var cl = cs.getCL( clName ) ;
      var conNum = inserNum/dNum ;
      for( var i = 0 ; i < groups.length ; ++i )
      {
         cl.split( csGroup[0], groups[i], { No : conNum } ) ;
         conNum = conNum * 2 ;
      }
   }
   catch( e )
   {
      println( "Failed to split, rc = " + e ) ;
      throw e ;
   }
}

// Insert data to SequoiaDB
function insertData( db, csName, clName, insertNum )
{
   if( undefined == insertNum ){ insertNum = 1000 ; }

   try
   {
      var cs = db.getCS( csName ) ;
      var cl = cs.getCL( clName ) ;
      for( var i = 0 ; i < insertNum ; ++i )
      {
         var no = i ;
         var user = "布斯"+i ;
         var phone = 13700000000+i ;
         var compa1 = "MI"+i ;
         var compa2 = "GOLDEN"+i ;
         var date = new Date() ;
         var time = date.getTime() ;
         var card = 6800000000+i ;
         /**********************************************************************
         data expampl : {"No":5, customerName:"布斯5","phoneNumber":13700000005,
                         "company":[MI5,GOLDEN5],"openDate":1402990912105,
                         "cardID":6800000005}
         **********************************************************************/
         //println( "Start to deal string" ) ;
         var insertString = "{\"No\":" + no + ",\"customerName\":\"" + user +
                            "\",\"phoneNumber\":" + phone +
                            ",\"company\":[\"" + compa1 + "\",\"" + compa2 +
                            "\"],\"openDate\":" + time + ",\"cardID\":" + card + "}" ;
         //println( "String + " + insertString ) ;
         var insert = eval("("+insertString+")") ;
         // insert date
         //println( "Begin to insert" ) ;
         cl.insert( insert ) ;
      }
      // inspect the data is insert success or not
      var cnt = 0 ;
      sleep( 20 ) ;
      do
      {
         var count = cl.count() ;
         if( insertNum == count )
            break ;
         ++cnt ;
         sleep( 100 ) ;
      }while( cnt < 100 ) ;
      if( insertNum != count )
      {
         println( "Wrong quantity of records, count = " + count +
                  " not equal insert number = " + insertNum ) ;
         throw "ErrNumRecord" ;
      }
      println( "Success to insert data. " ) ;
   }
   catch ( e )
   {
      println( "Failed to insert data to Sdb, rc = " + e ) ;
      throw e ;
   }
}

// Query the data
function queryData( db, csName, clName )
{
   try
   {
      var cs = db.getCS( csName ) ;
      var cl = cs.getCL( clName ) ;
      /**********************************************************************
      data expampl : {"No":5, customerName:"布斯5","phoneNumber":13700000005,
                      "company":[MI5,GOLDEN5],"openDate":1402990912105,
                      "cardID":6800000005}
      query data and get quantity.
      **********************************************************************/
      sleep( 20 ) ;
      var query =
      cl.find( {$and:[{ "No":{$gte:50}, "phoneNumber":{$lt:13700001000},
                        "customerName":{$nin:["MI500", "GOLDEN500"]},
                        "cardID":{$lte:6800001111},
                        "openDate":{$gt:1402990912105} }]}).count() ;
      if( 950 != query )
      {
         println( "Wrong query the data, count = " + query ) ;
         throw "ErrQueryNum" ;
      }
      println( "Success to query the data" ) ;
   }
   catch ( e )
   {
      println( "Failed to query data, rc = " + e ) ;
      throw e ;
   }
}

// Update the data
function updateData( db, csName, clName )
{
   try
   {
      var cs = db.getCS( csName ) ;
      var cl = cs.getCL( clName ) ;
      /**********************************************************************
      data expampl : {"No":5, customerName:"布斯5","phoneNumber":13700000005,
                      "company":[MI5,GOLDEN5],"openDate":1402990912105,
                      "cardID":6800000005}
      query data and get quantity.
      **********************************************************************/
      cl.update({ $inc:{"cardID":5}, $set:{"customerName":"布斯"},
                  $unset:{"openDate":""}, $addtoset:{company:[2,3]},
                  $pull_all:{"company":["MI88", "GOLDEN88",2,3]}}) ;
      sleep( 20 ) ;
      var count = cl.find({"No":88, "customerName":"布斯",
                           "phoneNumber":13700000088, "cardID":6800000093,
                           "company":["MI88", "GOLDEN88",2,3]}).count() ;
      if( 1 != count )
      {
         println( "Wrong update the data, count = " + count ) ;
         throw "ErrUpdateData" ;
      }
      println( "Success to update the data." ) ;
   }
   catch ( e )
   {
      println( "Failed to update the data, rc = " + e ) ;
      throw e ;
   }
}

// Remove data
function removeData( db, csName, clName )
{
   try
   {
      var cs = db.getCS( csName ) ;
      var cl = cs.getCL( clName ) ;
      /**********************************************************************
      data expampl : {"No":5, customerName:"布斯5","phoneNumber":13700000005,
                      "company":[MI5,GOLDEN5],"openDate":1402990912105,
                      "cardID":6800000005}
      query data and get quantity.
      **********************************************************************/
      cl.remove({"No":{$gte:89}}, {"":"$id"}) ;
      var cnt = 0 ;
      sleep( 20 ) ;
      do
      {
         var count = cl.count() ;
         if( 84 == count )
            break ;
         ++cnt ;
      }while( cnt <= 20 ) ;
      if( 89 != count )
      {
         println( "Wrong remove the date, count = " + count ) ;
         throw "ErrRemoveData" ;
      }
      println( "Success to remove the data." ) ;
   }
   catch ( e )
   {
      println( "Failed to remove the data, rc = " + e ) ;
      throw e ;
   }
}

// Get source group and destination group
function getTwoGroupSplit( db, csName, clName, splitArg1, splitArg2 )
{
   try
   {
      // get collection
      var cs = db.getCS( csName ) ;
      var cl = cs.getCL( clName ) ;

      var listGroups = db.listReplicaGroups() ;
      var listGroupsArr = new Array() ;

      // Check over arguement "splitArg1" "splitArg2"
      if( "" == splitArg1 || undefined == splitArg1 )
      {
         println( "Wrong argument." ) ;
         throw "ErrArg" ;
      }

      // argument : when the split is percent
      var argument = "" ;
      if( undefined == splitArg2 || "" == splitArg2 ){ argument = splitArg1 ; }
      // Get group where Collection Space located in
      while( listGroups.next() )
      {
         if ( listGroups.current().toObj()["GroupID"] >= DATA_GROUP_ID_BEGIN )
        {
           listGroupsArr.push( listGroups.current().toObj()["GroupName"] ) ;
        }
      }
      var groupNum = listGroupsArr.length ;

      var snapShotCl = db.snapshot( SDB_SNAP_COLLECTIONS ) ;
      var snapShotClName = new Array() ;
      var snapShotClGroup = new Array() ;
      var group = "" ;
      while( snapShotCl.next() )
      {
         snapShotClName.push( snapShotCl.current().toObj()["Name"] ) ;
         snapShotClGroup.push( snapShotCl.current().toObj()["Details"][0]["GroupName"] ) ;
      }
      var clname = csName + "." + clName ;
      for( var i=0 ; i<snapShotClGroup.length ; i++ )
      {
         if( snapShotClName[i] == clname )
         {
            group = snapShotClGroup[i] ;
            break ;
         }
      }
      if( "" == group )
      {
         println( "Failed to get Group where CL located in, snapshotCl = "
                  + snapShotCl ) ;
         throw "ErrGetGroup" ;
      }
      println( "The source group = " + group ) ;
      // Get the other group where split to
      var groupSplit = "" ;
      var i = 0 ;
      do
      {
         if( group != listGroupsArr[i] )
         {
            groupSplit = listGroupsArr[i] ;
            break ;
         }
         ++i ;

      }while( i <= groupNum || i <= 8 ) ;
      if( "" == groupSplit )
      {
         println( "Failed to get Split Group, Groups = " + listGroups ) ;
         throw "ErrGetSplitGroup" ;
      }
      println( "The destination [split]group = " + groupSplit ) ;
      //println( "Argument : " + argument ) ;
      if( "" == argument )
         cl.splitAsync( group, groupSplit, splitArg1, splitArg2 ) ;
      else
         cl.splitAsync( group, groupSplit, argument ) ;
      println( "Success to Split" ) ;

   }
   catch ( e )
   {
      println( "Failed to get the group " +e ) ;
      throw e ;
   }
}

// Get group from Sdb
function getGroup( db )
{
   try
   {
      var listGroups = db.listReplicaGroups() ;
      var groupArray = new Array() ;
      while( listGroups.next() )
      {
         if ( listGroups.current().toObj()["GroupID"] >= DATA_GROUP_ID_BEGIN )
         {
            groupArray.push( listGroups.current().toObj()["GroupName"] ) ;
         }
      }
      return groupArray ;
   }
   catch ( e )
   {
      println( "Failed to get groups from sdb, rc = " + e ) ;
      throw e ;
   }
}

