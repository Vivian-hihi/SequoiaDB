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
main();
function main()
{ 
   if( true == commIsStandalone( db ) )
   {
      println( "run mode is standalone" );
      return;
   }
   var clName = COMMCLNAME + "_IndexAndDataSyn_16997";
   commDropCL(db, COMMCSNAME, clName, true, true);
   var groups = getOneGroups(db);   
   var groupName = groups[0].GroupName;
   var options = {ReplSize:0,Compressed:true, Group:groupName};
   var dbcl = commCreateCLByOption(db, COMMCSNAME, clName, options);     
   
   dbcl.createIndex("idxfull",{'inta':"text",'str':"text"},true) 
   dbcl.createIndex("idxa",{'inta':1,'str':1},true);  
   dbcl.createIndex("idxb",{'fc':1},true);
   
   var expRecs = insertData( dbcl );  
   updateDatas( dbcl, expRecs);  
   getUpdateExpRecs(expRecs);
   
   dbcl.dropIndex("idxfull");
   
   // insert 2W records again, with a range of 20000-40000,eg:no:[20000,39999]
   var beginNo = 20000;
   var expRecsAfterInsert = buckInsertData( dbcl, 20000, beginNo);   
   
   var sortCond = {'inta':1};
   var expRecs = expRecs.concat(expRecsAfterInsert);   
   checkDataContent(db,COMMCSNAME, clName, sortCond, expRecs, "16997");        
   checkInspectResult(COMMCSNAME, clName);
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}

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

