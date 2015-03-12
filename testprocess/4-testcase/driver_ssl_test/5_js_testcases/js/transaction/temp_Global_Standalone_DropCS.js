/******************************************************************************
*@Description : drop collection space when in standalone mode
*@Modify list :
*               2014-11-10  xiaojun Hu  Change
******************************************************************************/
try
{
   var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
   if( false == commIsStandalone( db ) )
      throw "NotStandaloneMode" ;
   var cdbQuery = db.listCollectionSpaces(null,{"Name":""}) ;
   var csName = new Array() ;
   csName = cdbQuery.toArray() ;
   for ( var i = 0 ; i < csName.length ; ++i )
   {
      var name = eval( '(' +csName[i]+ ')' ) ;
      var cs = name["Name"] ;
      try
      {
         db.dropCS( cs ) ;
         println( "success to drop collection space : " + cs ) ;
      }
      catch ( e )
      {
         println( "can not to drop collection space : " + cs ) ;
         continue ;
      }
   }
}
catch( e )
{
   if( "NotStandaloneMode" != e )
   {
      println( "failed to drop collection space, rc = " + e ) ;
      throw e ;
   }
   else
      println( "run mode is standalone" ) ;
}
