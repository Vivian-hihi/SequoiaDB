/************************************
*@Description: seqDB-33914 GB18030È«Á¿×Ö·û¼¯²âÊÔ
*@author:      chenzejia
*@createDate:  2023.12.16
**************************************/
import( "./libs/grade1.js" );
import( "./libs/grade2.js" );
import( "./libs/grade3.js" );
import( "./libs/gb18030_data.js" );

main( test );
function test ()
{
   db.setCharsets( "GB18030" );
   var test_data = [];
   test_data.push( { _id: -1, a: single[0] } );
   for( var i = 0; i < 50; ++i )
   {
      test_data.push( { _id: i, a: double[random_number( double )] } );
      test_data.push( { _id: i + 50, a: four[random_number( four )] } );
      test_data.push( { _id: i + 100, a: cjk_grade1[random_number( cjk_grade1 )] } );
      test_data.push( { _id: i + 150, a: other_grade1[random_number( other_grade1 )] } );
      test_data.push( { _id: i + 200, a: grade2[random_number( grade2 )] } );
      test_data.push( { _id: i + 250, a: cjk_grade3[random_number( cjk_grade3 )] } );
      test_data.push( { _id: i + 300, a: kxbs[random_number( kxbs )] } );
   }
   test_data.sort( function( a, b ) { return a._id - b._id; } );
   test_all_characters( db );
   test_ddl( db, test_data, 10 );
   test_dml( db, test_data );
}

function random_number ( data )
{
   var random = Math.floor( Math.random() * data.length );
   return random;
}

function test_all_characters ( db )
{
   var csName = "cs_33914";
   var clName = "cl_33914";
   commDropCL( db, csName, clName );
   var cl = commCreateCL( db, csName, clName );
   // prepare data
   var test_data = []
   for( var i = 0; i < chinese_data.length; ++i )
   {
      test_data.push( { _id: i, a: chinese_data[i] } );
   }
   cl.insert( test_data );
   commCompareResults( cl.find().sort( { _id: 1 } ), test_data, false );
   commDropCS( db, csName );
}

function test_ddl ( db, test_data, rate )
{
   common_test( double )
   common_test( four )
   common_test( cjk_grade1 )
   common_test( other_grade1 )
   common_test( grade2 )
   common_test( cjk_grade3 )
   common_test( kxbs )
   function common_test ( array )
   {
      for( var i = 0; i < rate; ++i )
      {
         var csName = array[random_number( array )];
         var clName = array[random_number( array )];
         var indexName = array[random_number( array )];
         // create CS
         commCreateCS( db, csName );
         // create CL
         var cl = commCreateCL( db, csName, clName, {}, false );
         // create index
         commCreateIndex( cl, indexName, { a: 1 } );
         // insert data and check
         cl.insert( test_data );
         commCompareResults( cl.find(), test_data, false );
         // find with index
         var idx = random_number( test_data );
         var cursor = cl.find( { a: test_data[idx].a, _id: idx - 1 } ).hint( { "": indexName } );
         var expected = [test_data[idx]];
         commCompareResults( cursor, expected, false );
         // drop index
         commDropIndex( cl, indexName );
         assert.tryThrow( SDB_IXM_NOTEXIST, function()
         {
            cl.getIndex( indexName );
         } );
         // drop cl
         commDropCL( db, csName, clName );
         assert.tryThrow( SDB_DMS_NOTEXIST, function()
         {
            db.getCS( csName ).getCL( clName );
         } );
         // drop cs
         commDropCS( db, csName );
         assert.tryThrow( SDB_DMS_CS_NOTEXIST, function()
         {
            db.getCS( csName );
         } );
      }
   }
}

function test_dml ( db, test_data )
{
   var csName = "cs_33914";
   var clName = "cl_33914";
   var indexName = "index_33914";
   commDropCL( db, csName, clName );
   var cl = commCreateCL( db, csName, clName );
   commCreateIndex( cl, indexName, { a: 1 } );
   cl.insert( test_data );

   for( var i = 1; i < test_data.length; ++i )
   {
      // findWithRegex
      var regex = "^" + test_data[i]["a"] + "$";
      var res = cl.find( { _id: test_data[i]["_id"], a: { $regex: regex } } );
      var expected = [test_data[i]];
      commCompareResults( res, expected, false );

      // update
      cl.update( { $set: { a: "new_" + test_data[i]["a"] } }, { _id: test_data[i]["_id"], a: test_data[i]["a"] } );
      expected = [{ _id: test_data[i]["_id"], a: "new_" + test_data[i]["a"] }];
      res = cl.find( { _id: test_data[i]["_id"], a: "new_" + test_data[i]["a"] } );
      commCompareResults( res, expected, false );

      cl.update( { $push: { arr: test_data[i]["a"] } }, { _id: test_data[i]["_id"], a: "new_" + test_data[i]["a"] } );
      expected = [{ _id: test_data[i]["_id"], a: "new_" + test_data[i]["a"], arr: [test_data[i]["a"]] }];
      res = cl.find( { _id: test_data[i]["_id"], a: "new_" + test_data[i]["a"] } );
      commCompareResults( res, expected, false );

      // delete
      cl.remove( { a: "new_" + test_data[i]["a"] } );
      res = cl.find( { a: "new_" + test_data[i]["a"] } ).toArray();
      assert.equal( res.length, 0 );
   }
   commDropCS( db, csName );
}


