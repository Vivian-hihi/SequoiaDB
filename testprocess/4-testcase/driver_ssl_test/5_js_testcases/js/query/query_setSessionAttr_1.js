/*******************************************************************************
*@Description : test db.setSessionAttr({PreferedInstance:"M/S/m/s"}) function.
*               [BUG]JIRA_589
*@Modify list :
*               2015-2-10  xiaojun Hu  Init
*******************************************************************************/

function main( db )
{
   var insertNum = 100 ;
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, false,
                          "failed to create collection in the beignning" ) ;
   idxAutoGenData( cl, insertNum) ;
   println( "success to insert data, number = " + insertNum ) ;
   // make sure have enough group
   var clName = COMMCSNAME + "." + COMMCLNAME ;
   var group = commGetCLGroups( db, clName ) ;
   if( 1 < group.length )
   {
      println( "get group name : " + group ) ;
      throw "get more than one group" ;
   }
   var groups = commGetGroups( db ) ;
   for( var i = 0 ; i < groups.length ; ++i )
   {
      var rgName = groups[i][0]["GroupName"] ;
      //println( "rg name : " + rgName ) ;
      if( rgName == group[0] )
      {
         if( 1 < groups[i].length )
         {
            println( "group have enougth data node" ) ;
            break ;
         }
         else
         {
            println( "Warning, group only have one data node" ) ;
            return ;
         }
      }
      else
      {
         continue ;
      }
   }
   var rg = db.getRG( group[0] ) ;
   var master = rg.getMaster().toString().split( ":" ) ;
   var masterDB = new SecureSdb( master[0], master[1] ) ;
   var masterCS = masterDB.getCS( COMMCSNAME ) ;
   var masterCL = masterCS.getCL( COMMCLNAME ) ;
   if( insertNum != masterCL.count() )
   {
      println( "expect record number: " + insertNum +
               ", actual record number: " + masterCL.count() ) ;
      throw "NoExpectRecordInMaster" ;
   }
   masterCL.remove() ;   // remove data from master

   // set session attribution read from master['M']
   db.setSessionAttr( { PreferedInstance: "M"} ) ;
   if( 0 != cl.count() )
   {
      println( "actual record: " + cl.count() + " not equal 0" ) ;
      throw "1.Setting is not in effect" ;
   }
   println( "set session attribution read from master is in effect[M]" ) ;

   // set session attribution read from slave['S']
   db.setSessionAttr( { PreferedInstance: "S"} ) ;
   if( 100 != cl.count() )
   {
      println( "actual record: " + cl.count() + " not equal 100" ) ;
      throw "2.Setting is not in effect" ;
   }
   println( "set session attribution read from slave is in effect[S]" ) ;

   // set session attribution read from master['m']
   db.setSessionAttr( { PreferedInstance: "m"} ) ;
   if( 0 != cl.count() )
   {
      println( "actual record: " + cl.count() + " not equal 0" ) ;
      throw "3.Setting is not in effect" ;
   }
   println( "set session attribution read from master is in effect[m]" ) ;

   // set session attribution read from slave['s']
   db.setSessionAttr( { PreferedInstance: "s"} ) ;
   if( 100 != cl.count() )
   {
      println( "actual record: " + cl.count() + " not equal 100" ) ;
      throw "4.Setting is not in effect" ;
   }
   println( "set session attribution read from slave is in effect[s]" ) ;
}


try
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "clear collection in the beginning" ) ;
   if( false == commIsStandalone( db ) )
   {
      main( db ) ;
   }
   else
   {
      println( "run mode is standalone" ) ;
   }
   //commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
   //            "clear collection in the end, correct way" ) ;
   db.close() ;
}
catch( e )
{
   //commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
   //            "clear collection in the end, wrong way" ) ;
   db.close() ;
   throw e ;
}
