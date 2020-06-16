/******************************************** 
@description : insertOne/updateOne/findOne/deleteOne
@testcase    : seqDB-21965
@author      : XiaoNi Huang 2020-02-24
*********************************************/
main();

function main ()
{
   var clName = "cl21965";
   var cl = db.getCollection( clName );
   cl.drop();

   //cl.insertOne(<document>,{writeConcern:<document>})
   // insert document
   var rc = cl.insertOne( { "_id": 1, "a": 1, "b": "insertOne" } );
   assert.eq( rc, { "acknowledged": true, "insertedId": 1 } );

   // insert document, point writeConcern
   var rc = cl.insertOne( { "_id": 2, "a": 2, "b": "insertOne" }, { "writeConcern": { "w": 1 } } );
   assert.eq( rc, { "acknowledged": true, "insertedId": 2 } );

   // insert {}, random oid, delete after insert for check results
   var rc = cl.insertOne( {} );
   assert.eq( JSON.stringify( rc ).length, 37 );
   var oid = rc.insertedId;
   var rc = cl.deleteOne( { "_id": oid } );
   assert.eq( rc, { "acknowledged": true, "deletedCount": 1 } );


   /* TODO sdb not support
   // cl.updateOne(<filter>,<update>,{upsert:<boolean>,multi:<boolean>,writeConcern:<document>,collation:<document>})
   // updateOne, not support
   var rc = cl.updateOne( {}, { "$set": { "b": "update" } } );
   assert.eq( rc, { "acknowledged" : true, "matchedCount" : 1, "modifiedCount" : 1 } );
   // check results
   var rc = cl.find().sort( { "a": 1 } );
   checkResults( rc, expDocs );
   */


   /* TODO sdb not support
   // cl.findOne(<query>,<projection>)
   // not param
   var rc = cl.findOne();
   assert.eq( rc, { "_id" : 1, "a" : 1, "b" : "updateMany", "u3" : 1 } );
   // param: query
   var rc = cl.findOne( { "a": { "$gte": 10 } } );
   assert.eq( JSON.stringify( rc ), "{\"_id\":11,\"a\":11,\"b\":\"updateMany\",\"u3\":1}" );
   // param: query / projection
   var rc = cl.findOne( { "a": 1 }, { "a": "" } );
   assert.eq( rc, { "_id": 1, "a": 1 } );
   */


   /* TODO sdb not support
   // cl.deleteOne(<filter>,{writeConcern:<document>,collation:<document>})
   // param: filter
   var rc = cl.deleteOne( {} );
   assert.eq( rc, { "acknowledged": true, "deletedCount": 1 } );
   checkResults( cl.find(), ["[{\"_id\":42,\"a\":42,\"um1\":1}]"] );
   // all param
   var rc = cl.deleteOne( {}, { "writeConcern": { "j": true }, "collation": { "locale": "fr" } } );
   assert.eq( rc, { "acknowledged": true, "deletedCount": 1 } );
   checkResults( cl.find(), ["[]"] );
*/

   cl.drop();
}

function checkResults ( rc, expDocs )
{
   var docs = new Array();
   while( rc.hasNext() )
   {
      var doc = rc.next();
      docs.push( doc );
   }
   assert.eq( JSON.stringify( docs ), expDocs );
}