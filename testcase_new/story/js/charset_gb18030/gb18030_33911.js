/************************************
*@Description: seqDB-33911 设置字符集为GB18030，执行序列操作
*@author:      chenzejia
*@createDate:  2023.12.16
**************************************/
testConf.skipStandAlone = true;
main( test );
function test ()
{
   var sequenceName = "序列_33911";
   var newSequenceName = "新序列_33911";
   db.setCharsets( "GB18030" );

   // create sequence
   db.createSequence( sequenceName );

   // rename sequence
   db.renameSequence( sequenceName, newSequenceName );
   var cursor = db.listSequences();
   var sequenceNames = [];
   while( cursor.next() )
   {
      sequenceNames.push( cursor.current().toObj().Name );
   }
   cursor.close();
   assert.tryThrow( SDB_SEQUENCE_NOT_EXIST, function()
   {
      db.getSequence( sequenceName );
   } );
   if( sequenceNames.indexOf( newSequenceName ) < 0 )
   {
      throw new Error( "rename sequence failed" );
   }

   var seq = db.getSequence( newSequenceName );
   // check sequence
   var value = seq.getNextValue();
   assert.equal( 1, value );
   value = seq.getCurrentValue();
   assert.equal( 1, value );

   // drop sequence
   db.dropSequence( newSequenceName );
   assert.tryThrow( SDB_SEQUENCE_NOT_EXIST, function()
   {
      db.getSequence( newSequenceName );
   } );
}