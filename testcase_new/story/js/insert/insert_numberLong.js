/* *****************************************************************************
@Description : Insert data that field name have number long.
@Modify list :
               2016-3-29  wuyida  Init
***************************************************************************** */

var csName = COMMCSNAME ;
var clName = COMMCLNAME ;

function main( db )
{
   // drop cl
   commDropCL( db, csName, clName, true, true, "drop collection in begin" ) ;

   // create cl
   var varCL = commCreateCL( db, csName, clName, -1, false, true, false,
                             "create collection in begin" ) ;
   // special character put in array
   var longStr = "9223372036854775807 -9223372036854775807" ;
   var longChar = new Array() ;
   longChar = longStr.split(" ") ;

   // get the collection
   var cs = db.getCS( csName ) ;
   var cl = cs.getCL( clName ) ;

   // insert the data
   for( var i = 0 ; i < longChar.length ; ++i )
   {
      insertNumberLong( cl, longChar[i] )
   }

   // clean - drop cl
   commDropCS( db, csName, "drop CS in the end" )
}

// main entry
try
{
   main( db ) ;
   db.close() ;
}
catch( e )
{
   throw e ;
}

/*********************************************************************************
@Description : Insert number long chars in field name
@Author :      wuyida
@Parameter :
               key: Field name include special characters.
               value: common string type field value.
**********************************************************************************/
function insertNumberLong( cl, longStr )
{
   try
   {
      if( cl == "" || cl == undefined ||
          longStr == "" || longStr == undefined )
      {
         println( "Wrong argument." ) ;
         throw "ErrArg" ;
      }
	  var input = parseInt(longStr) ;
      var insertStr = "{ a: { $numberLong: \""+longStr+"\" } }" ;
      var evalInsertStr = eval("("+insertStr+")") ;
      cl.insert( evalInsertStr ) ;
      var query = cl.find( evalInsertStr ) ;
      var result = query.next() ;
	  var num = result.toObj().a ;
	  var expect = "\"$numberLong\": \""+longStr+"\"" ;
	  
	  if ( num.toString().indexOf(expect) == -1 )
	  {
        println( "Failed to inspect the data , exact: " + num.toString() + ", expect: " + expect ) ;
        throw "ErrNumberLong" ;	  
	  }
	  cl.remove() ;
   }
   catch ( e )
   {
      if( -6 != e )
      {
         println( "Failed to insert number long." +e  ) ;
         throw e ;
      }
   }
}

