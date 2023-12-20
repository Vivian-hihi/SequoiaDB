/************************************
*@Description: 设置字符集为GB18030，插入查询压缩数据
*@author:      chenzejia
*@createdate:  2023.12.16
*@testlinkCase:seqDB-33909
**************************************/

main( test );
function test ()
{
   db.setCharsets( "GB18030" );
   var csName = "cs_33909";
   var clName = "cl_33909";

   var data = [
      { str: "爯爲爳爴爺爼爾牀", arr: ["黄山", "泰山"], obj: { a: "衡山" } },
      { str: "狉狊狋狌狏", arr: ["广州", "上海"], obj: { a: "深圳" } },
      { str: "䵰䵱䵲䵳䵴", arr: ["衬衫", "T恤"], obj: { a: "裤子" } },
      { str: "䶛䶜䶝䶞䶟䶠䶡", arr: ["苹果", "香蕉"], obj: { a: "橘子" } },
      { str: "鿋鿌鿍鿎鿏鿐鿑鿒鿓鿔", arr: ["鸡蛋", "鸭蛋"], obj: { a: "鹅蛋" } },
      { str: "𫔶𫌀𫖳𫘪𫘬𫞩𡐓𪤗𣗋𬸘", arr: ["a", "", 123], obj: { a: "" } },
      { str: "𠅤𬇹𬍡", arr: [["嵌套"], "不嵌套"], obj: { a: { b: "崛起" } } },
      { str: "𫠋𫠌𫠍", arr: [1, { a: "东西" }], obj: { a: "南北" } },
      { str: "䰰䰱䰲䰳", arr: [[], "", null], obj: { a: ["上下", "左右"] } }
   ]

   // create not compressed cl
   commDropCL( db, csName, clName );
   var option = { Compressed: false };
   var cl = commCreateCL( db, csName, clName, option );
   cl.insert( data );
   commCompareResults( cl.find(), data, true );

   //create lzw compressed cl
   commDropCL( db, csName, clName );
   option = { Compressed: true, CompressionType: "lzw" };
   cl = commCreateCL( db, csName, clName, option );
   cl.insert( data );
   commCompareResults( cl.find(), data, true );

   //create snappy compressed cl
   commDropCL( db, csName, clName );
   option = { Compressed: true, CompressionType: "lzw" };
   cl = commCreateCL( db, csName, clName, option );
   cl.insert( data );
   commCompareResults( cl.find(), data, true );

   commDropCS( db, csName );
}