/************************************
*@Description: 设置字符集为GB18030，执行DML语句
*@author:      chenzejia
*@createdate:  2023.12.16
*@testlinkCase:seqDB-33906
**************************************/

main( test );
function test ()
{
   db.setCharsets( "GB18030" );
   var csName = "测试空间_33906";
   var clName = "集合_33906";
   var indexName = "索引_33906";
   var data = [
      // int,long,float,decimal,objId,binary
      { _id: 0, int: 123, long: { "$numberLong": "9223372036854775807" }, float: 123e+50, decimal: { $decimal: "123.456" }, objId: { "$oid": "123abcd00ef12358902300ef" }, binary: { "$binary": "aGVsbG8gd29ybGQ=", "$type": "1" } },
      // date,timestamp,bool,null,minkey,maxkey
      { _id: 1, date: { "$date": "2012-01-01" }, timestamp: { "$timestamp": "2012-01-01-13.14.26.124233" }, bool: true, null: null, minkey: { "$minKey": 1 }, maxkey: { "$maxKey": 1 } },
      // string,regex,array,object
      { _id: 2, str: "黟黢黩黧黥鼷鼽鼾齄", obj: { double: "犂犃", four: "㐟㐠", regex: { "$regex": "^龦龧", "$options": "i" } }, arr: ["齹齺", "㐕㐖㐙", "", "#$%"], regex: { "$regex": "^𬴂𫘦𫟅$", "$options": "i" } },
      { _id: 3, str: " 㐕㐖㐙 ", obj: { cjk: "𫝃𫝄𫝅", kxbs: "⽆⽇⽈" }, arr: ["⽘", "aA", "12", "@#￥%"], regex: { "$regex": "𠀛𠀜𠀝", "$options": "i" } },
      { _id: 4, str: "𨱔𬭼𫔎", obj: { a: { a: { "cjk": "𫞗𫞘𫞙𫞚", gfzb: "𫫇𫓹𬭚𬭛" }, b: { "$regex": "^W", "$options": "i" } } }, arr: ["1", "黄金万两", "一二", "叁肆"], regex: { "$regex": "𫝴𫝵$", "$options": "s" } },
      { _id: 5, str: "巨杉数据库", obj: { a: "魑魅魍魉" }, arr: [{ "$regex": "^中国", "$options": "i" }, { a: "你好", b: [1, "", "鬼神来渡"] }], regex: { "$regex": "^ab12$", "$options": "i" } },
      { _id: 6, str: "金牌 厨师", obj: { a: { "$maxKey": 1 }, b: { "$date": "2012-01-01" } }, arr: [{ "$binary": "aGVsbG8gd29ybGQ=", "$type": "1" }, null, false], regex: { "$regex": "?龻.*", "$options": "i" } }
   ];
   commDropCL( db, csName, clName );
   var cl = commCreateCL( db, csName, clName );
   commCreateIndex( cl, indexName, { str: 1 } );

   // insert data
   cl.insert( data );
   commCompareResults( cl.find(), data, false );

   // select
   // compare，sort
   var cond = { str: { $gt: "黄色" } };
   var cursor = cl.find( cond ).sort( { str: 1 } );
   var expect_result = [data[2], data[4]];
   commCompareResults( cursor, expect_result, false );
   // regex
   cond = { "obj.cjk": { $regex: "𫝃𫝄𫝅$" } };
   cursor = cl.find( cond );
   expect_result = [data[3]];
   commCompareResults( cursor, expect_result, false );
   cond = { str: { $regex: "^巨杉.*$" } };
   cursor = cl.find( cond );
   expect_result = [data[5]];
   commCompareResults( cursor, expect_result, false );
   // hint
   cond = { str: { $gt: "" } };
   cursor = cl.find( cond ).hint( { "": indexName } );
   expect_result = [data[3], data[5], data[6], data[2], data[4]];
   commCompareResults( cursor, expect_result, false );
   //aggregate
   cursor = cl.aggregate( { $match: { str: { $regex: ".*" } } },
      { $group: { _id: "$str", "str": { "$first": "$str" }, "arr": { "$push": "$obj" } } }
   );
   var expect_result = [
      { "str": " 㐕㐖㐙 ", arr: [{ "cjk": "𫝃𫝄𫝅", "kxbs": "⽆⽇⽈" }] },
      { "str": "巨杉数据库", arr: [{ "a": "魑魅魍魉" }] },
      { "str": "金牌 厨师", arr: [{ "a": { "$maxKey": 1 }, "b": { "$date": "2012-01-01" } }] },
      { "str": "黟黢黩黧黥鼷鼽鼾齄", arr: [{ "double": "犂犃", "four": "㐟㐠", "regex": { "$regex": "^龦龧", "$options": "i" } }] },
      { "str": "𨱔𬭼𫔎", arr: [{ "a": { "a": { "cjk": "𫞗𫞘𫞙𫞚", "gfzb": "𫫇𫓹𬭚𬭛" }, "b": { "$regex": "^W", "$options": "i" } } }] }
   ];
   commCompareResults( cursor, expect_result, false );

   // update
   // $set
   cl.update( { $set: { "string": { $field: "str" } } }, { str: "金牌 厨师" } );
   expect_result = [
      { _id: 6, arr: [{ "$binary": "aGVsbG8gd29ybGQ=", "$type": "1" }, null, false], obj: { a: { "$maxKey": 1 }, b: { "$date": "2012-01-01" } }, regex: { "$regex": "?龻.*", "$options": "i" }, str: "金牌 厨师", string: "金牌 厨师" }
   ];
   cursor = cl.find( { str: "金牌 厨师" } );
   commCompareResults( cursor, expect_result, false );
   // $unset
   cl.update( { $unset: { "arr.2": "" } }, { str: { $regex: "㐕㐖㐙" } } )
   expect_result = [
      { _id: 3, str: " 㐕㐖㐙 ", obj: { cjk: "𫝃𫝄𫝅", kxbs: "⽆⽇⽈" }, arr: ["⽘", "aA", null, "@#￥%"], regex: { "$regex": "𠀛𠀜𠀝", "$options": "i" } }
   ];
   cursor = cl.find( { str: { $regex: "㐕㐖㐙" } } );
   commCompareResults( cursor, expect_result, false );
   // $push
   cl.update( { $push: { arr: "䶣䶤" } }, { str: "𨱔𬭼𫔎" } );
   expect_result = [
      { _id: 4, str: "𨱔𬭼𫔎", obj: { a: { a: { "cjk": "𫞗𫞘𫞙𫞚", gfzb: "𫫇𫓹𬭚𬭛" }, b: { "$regex": "^W", "$options": "i" } } }, arr: ["1", "黄金万两", "一二", "叁肆", "䶣䶤"], regex: { "$regex": "𫝴𫝵$", "$options": "s" } }
   ];
   cursor = cl.find( { str: "𨱔𬭼𫔎" } );
   commCompareResults( cursor, expect_result, false );
   // $addToSet
   cl.update( { $addtoset: { arr: ["⽘", "东"] } }, { str: { $et: " 㐕㐖㐙 " } } );
   expect_result = [
      { _id: 3, str: " 㐕㐖㐙 ", obj: { cjk: "𫝃𫝄𫝅", kxbs: "⽆⽇⽈" }, arr: ["⽘", "aA", null, "@#￥%", "东"], regex: { "$regex": "𠀛𠀜𠀝", "$options": "i" } }
   ];
   cursor = cl.find( { str: { $regex: "㐕㐖㐙" } } );
   commCompareResults( cursor, expect_result, false );

   // delete
   cl.remove( { str: { $regex: ".*" } } );
   expect_result = [data[0], data[1]];
   commCompareResults( cl.find(), expect_result, false );

   commDropCS( db, csName );
}