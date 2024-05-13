/******************************************************************************
 * @Description   : seqDB-33950:netcompressor参数测试
 * @Author        : huangxiaoni
 * @CreateTime    : 2023.03.01
 * @LastEditTime  : 2024.05.11
 * @LastEditors   : huangxiaoni
                    wenjingwang
 ******************************************************************************/
testConf.skipStandAlone = true;

main( test );

function checkUpdateNetcompressor(value, expect, err)
{
   try{
      db.updateConf( { "netcompressor": value } );
      var cursor = db.snapshot( 13, {}, { netcompressor: "" } );
      assert.equal( cursor.current().toObj().netcompressor, expect );
      cursor.close();
      if ( typeof(err) !== "undefined" )
      {
         throw new Error("expect error:" + getErr(err));
      }
  }catch(e){
     if (typeof(err) !== "undefined"){
          assert.equal( e, err ); 
     }else{
        throw e;
     }
  }
}

function test ()
{
   try
   {
      // netcompressor有效参数校验
      var validValues = ["", "lz4", "LZ4", "Lz4", "lZ4"];
      for( var i = 0; i < validValues.length; i++ )
      {
         checkUpdateNetcompressor(validValues[i],validValues[i]);
      }
    
    // netcompressor无效参数校验，不报错也不生效
    var invalidValues = ["snappy", "lzw", "lz"];
    for( var i = 0; i < invalidValues.length; i++ )
      {
        checkUpdateNetcompressor(invalidValues[i],"");
    }
    // netcompressor值类型不对，报参数错误
    checkUpdateNetcompressor(null, "", -6);
   }
   finally
   {
      db.updateConf( { "netcompressor": "" } );
   }
}
