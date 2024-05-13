/******************************************************************************
 * @Description   : netcompressor public function
 * @Author        : huangxiaoni
 * @CreateTime    : 2024.03.01
 * @LastEditTime  : 2024.05.10
 * @LastEditors   : huangxiaoni
                    wenjingwang
 ******************************************************************************/
import( "../lib/main.js" );
import( "../lib/basic_operation/commlib.js" );

function setUp(testpara)
{
   testpara.normalCL = commCreateCL(db, testpara.csName, testpara.normalCLName, { "ReplSize": -1 }, true, true);
   testpara.shardCL = commCreateCL(db, testpara.csName, testpara.shardCLName, { "ShardingKey": { "a": 1 }, AutoSplit:true,"ReplSize": -1 },true, true );
}

function insertHighDuplicateDocs ( cl )
{
   var docs = []
   for( var i = 0; i < 1000; i++ )
   {
      docs.push( { "a": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" + i, b:i} );
   }

   cl.insert(docs);

   return docs;
}

function insertLowDuplicateDocs ( cl )
{
   var docs = []
   for( var i = 0; i < 1000; i++ )
   {
      docs.push( { "a": "test" + i, "b":  i, "c": i } );
   }

   cl.insert(docs);
   return docs;
}

function findAndcheckResult ( cl, expectResult )
{
   assert.equal( cl.count(), expectResult.length );

   var cursor = cl.find().sort({b:1});
   commCompareResults (cursor, expectResult, true) ;
}
