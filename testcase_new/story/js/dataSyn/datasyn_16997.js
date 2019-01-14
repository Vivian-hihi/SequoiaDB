/************************************
*@Description: seqDB-16997 create multiple unique indexes and fullText,do the following:
               1.insert data 
               2.update data
               3.delete the fullText
               4.insert data
               5.check data synchronization                             
*@author:      wuyan
*@date:        2018.12.28
**************************************/
var csName = COMMCSNAME ;
var clName = COMMCLNAME + "_IndexAndDataSyn_16997";
DataSyncTestCase.prototype.execTest = function()
{   
   this.dbcl.createIndex("idxfull",{'inta':"text",'str':"text"},true) 
   this.dbcl.createIndex("idxa",{'inta':1,'str':1},true);  
   this.dbcl.createIndex("idxb",{'fc':1},true);
   
   var expRecs = insertData( this.dbcl );  
   updateDatas( this.dbcl, expRecs);  
   getUpdateExpRecs(expRecs);  
   this.dbcl.dropIndex("idxfull");  
  
   // insert 2W records again, with a range of 20000-40000,eg:no:[20000,39999]
   var beginNo = 20000;
   var expRecsAfterInsert = buckInsertData( this.dbcl, 20000, beginNo);   
   
   var sortCond = {'inta':1};
   var expRecs = expRecs.concat(expRecsAfterInsert); 
   this.checkResult( sortCond, expRecs, "16997");
}
main();

function updateDatas( dbcl, expRecs)
{   
   println("---begin to update data.");
   dbcl.update({ $set: { 'str': "test16997" } } );
   for( var i = 0 ; i < expRecs.length; i++ )
   {
      dbcl.update({ $inc: { 'no': 200000 } }, { 'inta': i});
   }
}

function getUpdateExpRecs(expRecs)
{
   for( var i = 0 ; i < expRecs.length; i++ )
   {
      var item = expRecs[i];
      item.str = "test16997";
      item.no = i + 200000;
   }   
   return expRecs;
}

