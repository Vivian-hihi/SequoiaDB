/******************************************** 
@description : auto create cs/cl
@testcase    : seqDB-21880
@author      : XiaoNi Huang 2020-02-24
*********************************************/
main();

function main ()
{
   var clName = "cl21880";
   var cl = db.getCollection( clName );
   db.dropDatabase();

   // cs not exist, auto create cs
   // createCollection
   db.createCollection( clName );
   var cl2 = db.getCollection( clName );
   assert.eq( cl2.count(), 0 );


   // cs not exist, auto create cs and cl
   // insert
   db.dropDatabase();
   cl.insert( { "a": 1 } );
   assert.eq( cl.count(), 1 );

   // update
   db.dropDatabase();
   cl.update( {}, { "$set": { "a": 1 } }, { "upsert": true } );
   assert.eq( cl.count(), 1 );

   // createIndex
   db.dropDatabase();
   cl.createIndex( { a: 1 } );
   assert.eq( cl.getIndexes().length, 2 );


   // cs exist, cl not exist, auto create cl
   // insert
   cl.drop();
   cl.insert( { "a": 1 } );
   assert.eq( cl.count(), 1 );

   // update
   cl.drop();
   cl.update( {}, { "$set": { "a": 1 } }, { "upsert": true } );
   assert.eq( cl.count(), 1 );

   // createIndex
   cl.drop();
   var cl = db.getCollection( clName );
   cl.createIndex( { a: 1 } );
   assert.eq( cl.getIndexes().length, 2 );


   // repeat createCollection
   var rc = db.createCollection( clName );
   assert.eq( JSON.stringify( rc ), ["{\"ok\":0,\"code\":-22,\"errmsg\":\"Collection already exists\"}"] );


   cl.drop();
}