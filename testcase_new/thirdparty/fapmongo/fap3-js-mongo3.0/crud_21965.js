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

   //ready data
   cl.insert([{"_id":1,"a":1},{"_id":2,"a":2}]);
   
   // cl.findOne(<query>,<projection>)
   // not param
   var rc = cl.findOne();
   assert.eq( rc, { "_id": 1, "a": 1 } );

   // param: query
   var rc = cl.findOne( { "a": { "$gte": 2 } } );
   assert.eq( rc, { "_id": 2, "a": 2 } );

   // param: query / projection
   var rc = cl.findOne( { "a": 1 }, { "a": "" } );
   assert.eq( rc, { "_id": 1, "a": 1 } );
   
   // not match docs
   var rc = cl.findOne( { "a": "notExist" } );
   assert.eq( rc, null );


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