/***************************************************************************
@Description : 13 bson type data inserted. Then create index .
@Modify list :
              2014-5-21  xiaojun Hu  Init
****************************************************************************/
function main( db )
{
   // drop collection in the beginning
   commDropCL( db, csName, clName, true, true,
               "drop collection in the beginning" ) ;

   // create collection
   var idxCL = commCreateCL( db, csName, clName, -1, true, true, false,
                             "create collection" ) ;

   // insert data to SDB
   try
   {
     idxCL.insert({number:24523453, longint:2147483647000, floatNum:12345.456}) ;
     idxCL.insert({string:"field_value", objectOID:{"$oid":"123abcd00ef12358902300ef"}}) ;
     idxCL.insert({floatNum:123e+50, bool:true,date:{"$date":"2014-5-21"}}) ;
     idxCL.insert({timestamp:{"$timestamp":"2014-5-21-9.17.30.111111"}}) ;
     idxCL.insert({binary:{"$binary":"aGVsbG8gd29ybGQ=", "$type":"1"}}) ;
     idxCL.insert({regex:{"$regex":"^张","$options":"i"}}) ;
     idxCL.insert({object:{"subobj":"can't"}}) ;
     idxCL.insert({array:["abc",123,"def","噆"], NULL:null}) ;
     var i = 0 ;
     do
     {
        var count = idxCL.count() ;
        ++i ;
     }while( i < 15 )
     if ( 8 != count )
     {
        println( "Wrong number of the insert record." ) ;
        throw "ErrNumRecord" ;
     }
   }
   catch ( e )
   {
      println( "Failed to insert data to SDB, rc="+e ) ;
      throw e ;
   }

   //createIndex
   createIndex( idxCL, "numberIdx", {number:1} );
   createIndex( idxCL, "longinIdx", {longint:1} ) ;
   createIndex( idxCL, "floatNumIdx", {floatNum:1} ) ;
   createIndex( idxCL, "stringIdx", {string:1} ) ;
   createIndex( idxCL, "objIDIdx", {"objectOID":-1} ) ;
   createIndex( idxCL, "boolIdx", {bool:-1} ) ;
   createIndex( idxCL, "dateIdx", {"date":-1} ) ;
   createIndex( idxCL, "timestampIdx", {"timestamp":-1} ) ;
   createIndex( idxCL, "binaryIdx", {"binary":-1} ) ;
   createIndex( idxCL, "objIdx", {"object.subobj":-1} ) ;
   createIndex( idxCL, "arrayIdx", {"array":-1} ) ;
   createIndex( idxCL, "nullIdx", {"NULL":-1} ) ;
   createIndex( idxCL, "regexIdx",{"regex":-1}) ;

   // inspect the index
   try
   {
      inspecIndex( idxCL, "numberIdx", "number", 1, false, false ) ;
      inspecIndex( idxCL, "longinIdx", "longint", 1, false, false ) ;
      inspecIndex( idxCL, "floatNumIdx", "floatNum", 1, false, false ) ;
      inspecIndex( idxCL, "stringIdx", "string", 1, false, false ) ;
      inspecIndex( idxCL, "objIDIdx", "objectOID", -1, false, false ) ;
      inspecIndex( idxCL, "boolIdx", "bool", -1, false, false ) ;
      inspecIndex( idxCL, "dateIdx", "date", -1, false, false ) ;
      inspecIndex( idxCL, "timestampIdx", "timestamp", -1, false, false ) ;
      inspecIndex( idxCL, "binaryIdx", "binary", -1, false, false ) ;
      inspecIndex( idxCL, "objIdx", "object.subobj", -1, false, false ) ;
      inspecIndex( idxCL, "arrayIdx", "array", -1, false, false ) ;
      inspecIndex( idxCL, "nullIdx", "NULL", -1, false, false ) ;
      inspecIndex( idxCL, "regexIdx", "regex", -1, false, false ) ;
   }
   catch ( e )
   {
      if ( "ErrIdxName" != e )
      {
         throw e ;
      }
   }

   // drop collectionspace in clean
   commDropCS( db, csName, true, "drop CS in clean " )
}

try
{
   main( db ) ;
   db.close() ;
}
catch ( e )
{
   throw e ;
}

