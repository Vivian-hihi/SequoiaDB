/******************************************************************************
 * @Description   : seqDB-33950:netcompressor参数测试
 * @Author        : huangxiaoni
 * @CreateTime    : 2023.03.01
 * @LastEditTime  : 2023.03.01
 * @LastEditors   : huangxiaoni
 ******************************************************************************/
testConf.skipStandAlone = true;

main( test );
function test ()
{
   // 初始化：关闭消息压缩
   db.updateConf( { "netcompressor": "" } );

   try
   {
      // netcompressor有效参数校验
      var validCompressor = ["", "lz4", "LZ4", "Lz4", "lZ4"];
      for( var i = 0; i < validCompressor.length; i++ )
      {
         db.updateConf( { "netcompressor": validCompressor[i] } );
         var cursor = db.snapshot( 13, {}, { netcompressor: "" } );
         assert.equal( cursor.current().toObj().netcompressor, validCompressor[i] );
         cursor.close();
      }
   }
   finally
   {
      db.updateConf( { "netcompressor": "" } );
   }

   // netcompressor无效参数校验，不报错也不生效
   var invalidCompressor = ["snappy", "lzw", "lz"];
   var cursor = db.snapshot( 13, {}, { netcompressor: "" } );
   assert.equal( cursor.current().toObj().netcompressor, "" );
   cursor.close();

   // netcompressor无效参数校验，报错
   var invalidCompressor = [null, 123];
   for( var i = 0; i < invalidCompressor.length; i++ )
   {
      try
      {
         db.updateConf( { "netcompressor": invalidCompressor[i] } );
      }
      catch( e )
      {
         assert.equal( e, -6 );
      }
   }
}