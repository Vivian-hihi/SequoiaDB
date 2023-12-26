/************************************
*@Description: seqDB-33910 设置字符集为GB18030，执行内置SQL语句
*@author:      chenzejia
*@createDate:  2023.12.16
**************************************/

main( test );
function test ()
{
   db.setCharsets( "GB18030" );
   var csName = "集合空间_33910";
   var clName = "集合_33910";
   var fullClName = csName + "." + clName;
   var indexName = "索引_33910";
   var data = [
      "爯爾牀", "䶛䶟䶠䶡", "鿋鿌鿑鿒鿓", "𫌀𫖳𫘪𫘬𫞩", "", "䰰䰱䰲䰳", "ab123#$%", "如履薄冰"
   ]

   commDropCS( db, csName );
   // create cs
   var sql = "create collectionspace " + csName;
   var rc = db.execUpdate( sql );
   assert.equal( rc, undefined );

   // create cl
   sql = "create collection " + fullClName;
   rc = db.execUpdate( sql );
   assert.equal( rc, undefined );

   // create index
   sql = "create index " + indexName + " on " + fullClName + "(a)";
   db.execUpdate( sql );
   var indexInfo = db.getCS( csName ).getCL( clName ).getIndex( indexName ).toObj();
   assert.equal( indexInfo.IndexDef.name, indexName );
   assert.equal( indexInfo.IndexDef.key, { "a": 1 } );

   // insert data
   for( var i = 0; i < data.length; i++ )
   {
      sql = "insert into " + fullClName + "(a) values(\"" + data[i] + "\")";
      db.execUpdate( sql );
   }
   // select
   // order by,limit,offset
   sql = "select a from " + fullClName + " order by a limit 2 offset 1";
   var expect_result = [
      { "a": "ab123#$%" },
      { "a": "䰰䰱䰲䰳" }
   ];
   commCompareResults( db.exec( sql ), expect_result, true );
   // group by
   sql = "select a from " + fullClName + " group by a";
   expect_result = [
      { "a": "" },
      { "a": "ab123#$%" },
      { "a": "䰰䰱䰲䰳" },
      { "a": "䶛䶟䶠䶡" },
      { "a": "如履薄冰" },
      { "a": "爯爾牀" },
      { "a": "鿋鿌鿑鿒鿓" },
      { "a": "𫌀𫖳𫘪𫘬𫞩" }
   ];
   commCompareResults( db.exec( sql ), expect_result, true );
   // join
   sql = "select t1.a as 值1,t2.a as 值2 from " + fullClName + " as t1 inner join " + fullClName + " as t2 on t1.a=t2.a";
   expect_result = [
      { "值1": "爯爾牀", "值2": "爯爾牀" },
      { "值1": "䶛䶟䶠䶡", "值2": "䶛䶟䶠䶡" },
      { "值1": "鿋鿌鿑鿒鿓", "值2": "鿋鿌鿑鿒鿓" },
      { "值1": "𫌀𫖳𫘪𫘬𫞩", "值2": "𫌀𫖳𫘪𫘬𫞩" },
      { "值1": "", "值2": "" },
      { "值1": "䰰䰱䰲䰳", "值2": "䰰䰱䰲䰳" },
      { "值1": "ab123#$%", "值2": "ab123#$%" },
      { "值1": "如履薄冰", "值2": "如履薄冰" },
   ];
   commCompareResults( db.exec( sql ), expect_result, true );
   // like
   sql = "select a from " + fullClName + " where a like '𫖳𫘪𫘬'";
   expect_result = [
      { "a": "𫌀𫖳𫘪𫘬𫞩" }
   ];
   commCompareResults( db.exec( sql ), expect_result, true );
   // function
   sql = "select max(a) as 最大值 from " + fullClName;
   expect_result = [
      { "最大值": "𫌀𫖳𫘪𫘬𫞩" }
   ];
   commCompareResults( db.exec( sql ), expect_result, true );

   // update
   sql = "update " + fullClName + " set a='1a更新数据' where a>''";
   db.execUpdate( sql );
   sql = "select a from " + fullClName + " order by a";
   expect_result = [
      { "a": "" },
      { "a": "1a更新数据" },
      { "a": "1a更新数据" },
      { "a": "1a更新数据" },
      { "a": "1a更新数据" },
      { "a": "1a更新数据" },
      { "a": "1a更新数据" },
      { "a": "1a更新数据" }
   ];
   commCompareResults( db.exec( sql ), expect_result, true );

   // delete
   sql = "delete from " + fullClName + " where a='1a更新数据'";
   db.execUpdate( sql );
   sql = "select a from " + fullClName + " order by a";
   expect_result = [
      { "a": "" }
   ];
   commCompareResults( db.exec( sql ), expect_result, true );

   // drop index
   sql = "drop index " + indexName + " on " + fullClName;
   db.execUpdate( sql );
   assert.tryThrow( SDB_IXM_NOTEXIST, function()
   {
      db.getCS( csName ).getCL( clName ).getIndex( indexName );
   } );

   // drop cl
   sql = "drop collection " + fullClName;
   db.execUpdate( sql );
   assert.tryThrow( SDB_DMS_NOTEXIST, function()
   {
      db.getCS( csName ).getCL( clName );
   } );

   // drop cs
   sql = "drop collectionspace " + csName;
   db.execUpdate( sql );
   assert.tryThrow( SDB_DMS_CS_NOTEXIST, function()
   {
      db.getCS( csName );
   } );
}