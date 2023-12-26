/************************************
*@Description: seqDB-33902 通过sdb方法设置字符集
*@author:      chenzejia
*@createDate:  2023.12.16
**************************************/

main( test );
function test ()
{
   function checkCharset ( clientCharset, resultsCharset )
   {
      var attr = db.getSessionAttr();
      var actClientCharset = attr.toObj().ClientCharset;
      var actResultsCharset = attr.toObj().ResultsCharset;
      assert.equal( actClientCharset, clientCharset, "check clientCharset failed,expect " + clientCharset + " but actually " + actClientCharset );
      assert.equal( actResultsCharset, resultsCharset, "check resultsChatset failed,expect " + resultsCharset + " but actually " + actResultsCharset );
   }

   // reset the default charset
   db.setCharsets( "UTF8" )
   checkCharset( "UTF8", "UTF8" );

   // set charset by setCharsets()
   db.setCharsets( "GB18030" );
   checkCharset( "GB18030", "GB18030" );
   db.setCharsets( "utf8" );
   checkCharset( "UTF8", "UTF8" );

   // set clientCharset by setClientCharset()
   db.setClientCharset( "gb18030" );
   checkCharset( "GB18030", "UTF8" );
   db.setClientCharset( "UTF8" );
   checkCharset( "UTF8", "UTF8" );
   // set resultsCharset by setResultsCharset()
   db.setResultsCharset( "gb18030" );
   checkCharset( "UTF8", "GB18030" );
   db.setResultsCharset( "utf8" );
   checkCharset( "UTF8", "UTF8" );

   // set charset by setSessionAttr()
   db.setSessionAttr( { "clientCharset": "gb18030" } );
   checkCharset( "GB18030", "UTF8" );
   db.setSessionAttr( { "clientCharset": "UTF8" } );
   checkCharset( "UTF8", "UTF8" );
   db.setSessionAttr( { "resultsCharset": "GB18030" } );
   checkCharset( "UTF8", "GB18030" );
   db.setSessionAttr( { "resultsCharset": "utf8" } );
   checkCharset( "UTF8", "UTF8" );

   // set invalid charset
   assert.tryThrow( SDB_INVALIDARG, function()
   {
      db.setCharsets( "gbk" );
   } );
   assert.tryThrow( SDB_INVALIDARG, function()
   {
      db.setClientCharset( "gbk" );
   }
   );
   assert.tryThrow( SDB_INVALIDARG, function()
   {
      db.setResultsCharset( "gbk" );
   }
   );
   assert.tryThrow( SDB_INVALIDARG, function()
   {
      db.setSessionAttr( { "clientCharset": "gbk" } );
   }
   );
}