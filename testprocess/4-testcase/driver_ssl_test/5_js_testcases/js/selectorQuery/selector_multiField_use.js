/*******************************************************************************
*@Description : when the number of fields greater than 65, we use selector to
*               query. such as: $include/$default/$slice/$elemMatchOne/$elemMatch
*@Modify list :
*               2015-01-29  xiaojun Hu  Change
*******************************************************************************/
// auto generate data, which have 65 fields
var fieldNum = 70 ;
function autoGenData( cl, recordNum )
{
   try
   {
      for( var i = 0 ; i < recordNum ; ++i )
      {
         var record = "{" ;
         var field = "" ;
         for( var j = 0 ; j < fieldNum ; ++j )
         {
            if( 0 == j )
               field = "\"field_" + j + "\":\"" + i*j + "_value\"" ;
            else
               field = ", \"field_" + j + "\":\"" + i*j + "_value\"" ;
            record = record + field  ;
         }
         record = record + "}" ;
         //println( "record : " + record ) ;
         cl.insert( JSON.parse( record ) ) ;
      }
      var cnt = 0 ;
      while( recordNum != cl.count() && 1000 > cnt )
      {
         cnt++ ;
         sleep( 3 ) ;
      }
      if( recordNum != cl.count() )
      {
         println( "expect record number: " + recordNum +
                  ", actual record number: " + cl.count() ) ;
         throw "ErrorNumer" ;
      }
   }
   catch( e )
   {
      throw e ;
   }
}
// auto generate selector for query
// such as : {"field_0":{$include:1},...,"field_n":{$include:1}}
function autoGenSelector( selField, selValue )
{
   var selQuery = "{" ;
   for( var i = 0 ; i < (fieldNum-2) ; ++i )
   {
      if( 0 == i )
         selVar = "\"field_" + i + "\":{\"" + selField + "\":" + selValue + "}" ;
      else
         selVar = ", \"field_" + i + "\":{\"" + selField + "\":" + selValue + "}" ;
      selQuery = selQuery + selVar ;
   }
   selQuery = selQuery + "}" ;
   return selQuery ;
}

function main( db )
{
   try
   {
      var recordNum = 100 ;
      var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, false,
                             "create colleciton in the begnning" ) ;
      // auto generate data
      autoGenData( cl, recordNum ) ;
      println( "success to insert record: " + recordNum ) ;

      /*【Test Point 1】$include=1/0*/
      var selField = "$include" ;
      var selValue = 1 ;
      var genSelector = autoGenSelector( selField, selValue ) ;
      println( "auto data: " + genSelector ) ;
      var condObj = {} ;
      var selObj = JSON.parse( genSelector ) ;
      var ret = selMainQuery( cl, condObj, selObj ) ;
      println( "==>success to test use: " + JSON.stringify( selObj ) ) ;
      selValue = 0 ;
      genSelector = autoGenSelector( selField, selValue ) ;
      condObj = {} ;
      selObj = JSON.parse( genSelector ) ;
      var ret = selMainQuery( cl, condObj, selObj ) ;
      println( "==>success to test use: " + JSON.stringify( selObj ) ) ;
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
