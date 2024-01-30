/*
 * @Description   : seqDB-33979:NAN/min/max/inf类型，读数据，覆盖表扫描和索引扫描
 * @Author        : huangxiaoni
 * @CreateTime    : 2024-01-25
 * @LastEditors   : huangxiaoni
 * @LastEditTime  : 2024-01-26
 */

main( test );
function test ()
{
   var csName = COMMCSNAME;
   var clName1 = CHANGEDPREFIX + "_33979_1";
   var clName2 = CHANGEDPREFIX + "_33979_2";
   var indexName = "idx_33979";

   commDropCL( db, csName, clName1, true, true, "drop cl in the beginning" );
   commDropCL( db, csName, clName2, true, true, "drop cl in the beginning" );

   var cl1 = commCreateCL( db, csName, clName1 );
   cl1.createIndex( indexName, { a: 1 } );
   var docs1 = [
      { a: -Infinity },
      { a: { $decimal: "MAX" } },
      { a: { $decimal: "MIN" } },
      { a: { $decimal: "NaN" } },
      { a: NaN },
      { a: Infinity }
   ];
   cl1.insert( docs1 );

   var cl2 = commCreateCL( db, csName, clName2 );
   cl2.createIndex( indexName, { a: 1 } );
   var docs2 = [
      { a: -Infinity },
      { a: { $decimal: "MAX" } },
      { a: { $decimal: "MIN" } },
      { a: { $decimal: "NaN" } },
      { a: NaN },
      { a: Infinity }
   ];
   cl2.insert( docs2 );

   var cursor1 = cl1.find( {}, { a: "" } ).sort( { a: 1 } ).hint( { "": "a" } );
   var cursor2 = cl2.find( {}, { a: "" } ).sort( { a: 1 } ).hint( { "": "a" } );
   var expDocs1 = getExpectedDocs( cursor1 );
   var expDocs2 = getExpectedDocs( cursor2 );
   assert.equal( expDocs1, expDocs2 );

   var cursor1 = cl1.find( { a: NaN }, { a: "" } );
   var cursor2 = cl1.find( { a: NaN }, { a: "" } ).hint( { "": null } );
   var expDocs1 = getExpectedDocs( cursor1 );
   var expDocs2 = getExpectedDocs( cursor2 );
   assert.equal( expDocs1, expDocs2 );

   commDropCL( db, csName, clName1, false, false );
   commDropCL( db, csName, clName2, false, false );
}

function getExpectedDocs ( cursor )
{
   var expectedResults = [];
   var doc;
   while( ( doc = cursor.next() ) )
   {
      expectedResults.push( doc );
   }
   return expectedResults;
}