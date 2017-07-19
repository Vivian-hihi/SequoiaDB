/************************************
*@Description:capped cl, repeate insert and pop, direciton set 1 
*@author:      zhaoyu
*@createdate:  2017.7.11
*@testlinkCase: seqDB-11795
**************************************/
function main()
{
   var csName = COMMCSNAME + "_11795";
   commDropCS( db, csName, true, "drop CS in the beginning" );
   
   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );
   
   var clName = COMMCLNAME + "_11795";
   var clOption = {Capped:true, Size:1073741824, AutoIndexId:false};
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true, true );
   
   var repeatedTimes = 10;
   var minLength = 0;
   var maxLength = 2048;
   var string = "a";
   repeatedInsertAndPopLastRecord( dbcl, repeatedTimes, minLength, maxLength, string );
   println("--end insert and check data");
   
   commDropCS( db, csName, true, "drop CS in the end" );
}

function repeatedInsertAndPopLastRecord( dbcl, repeatedTimes, minLength, maxLength, string )
{
   var preLogicalID = 0;
   try
   {
      //repeat pop and insert,check LogicalID is the same
      for(var i = 1 ; i < repeatedTimes; i++)
      {
         //insert record;
         var doc = new StringBuffer();
         var range = maxLength - minLength;
         var stringLength = minLength + parseInt( Math.random() * range );
         doc.append(stringLength, string);
         var strings = doc.toString();
         var recs = [{a:strings}];
         dbcl.insert(recs); 
         var record = dbcl.find().sort({_id:-1}).limit(1);
         while(record.next())
         {
            //check logicalID
            var logicalID = record.current().toObj()._id;
            var expectLogicalID = preLogicalID;
            if(logicalID !== expectLogicalID )
            {
               println("actual logicalID: " + logicalID + "\nexpect logicalID: " + expectLogicalID);
               throw "LOGICAL_ID_CHECK_ERROR";
            }
            
            //check record
            checkRecords( dbcl, null, null, null, null, null, recs );
         }
         
         var recordLength = stringLength + 55;
         if(recordLength % 4 !== 0)
         {
            recordLength = recordLength + (4 - recordLength % 4);
            
         }
         preLogicalID = logicalID + recordLength;
         
         dbcl.pop({LogicalID:logicalID,Direction:1});
      }
   }catch(e)
   {
      throw buildException("repeatedInsertAndPopLastRecord", e, null, null, e);
   }
}

main();



