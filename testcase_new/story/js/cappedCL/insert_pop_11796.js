/************************************
*@Description:capped cl find use sort/limit/skip 
*@author:      zhaoyu
*@createdate:  2017.7.11
*@testlinkCase: seqDB-11796
**************************************/
function main()
{
   var csName = COMMCSNAME + "_11796";
   commDropCS( db, csName, true, "drop CS in the beginning" );
   
   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );
   
   var clName = COMMCLNAME + "_11796";
   var clOption = {Capped:true, Size:1073741824, AutoIndexId:false};
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true, true );
   
   var stringLength = 969;
   var string = "a";
   recordNum = 32767;
   insertDatas( dbcl, recordNum, stringLength, string );
   
   var repeatedTimes = 100;
   repeatedInsertAndPopLastRecord( dbcl, repeatedTimes, stringLength, string );
   
   commDropCS( db, csName, true, "drop CS in the end" );
}

function StringBuffer()
{
   this._strings = new Array();
}
StringBuffer.prototype.append = function( stringLength, string ){
   for(var i = 0; i< stringLength; i++)
   {
      this._strings.push(string);   
   }
};
StringBuffer.prototype.toString = function(){
   return this._strings.join("");
};
StringBuffer.prototype.clear = function(){
   this._strings = [];
}
StringBuffer.prototype.size = function(){
   return this._strings.length;
}
   
function insertDatas( dbcl, recordNum, stringLength, string )
{
   try
   {
      var doc = new StringBuffer();
      doc.append(stringLength, string);
      var strings = doc.toString(); 
      
      var record = [];
      for(var i = 0; i < recordNum; i++)
      {
         record.push({a:strings});
      }
      dbcl.insert( record );
   }catch(e)
   {
      throw buildException("insertDatas", e, null, "insert datas success", e);
   }
   
}

function repeatedInsertAndPopLastRecord( dbcl, repeatedTimes, stringLength, string )
{
   try
   {
      //get the first logicalID
      var firstRecord = dbcl.find().sort({_id:-1}).limit(1);
      while(firstRecord.next())
      {
         var firstLogicalID = firstRecord.current().toObj()._id
      }
      
      //repeat pop and insert,check LogicalID is the same
      for(var i = 0 ; i < repeatedTimes; i++)
      {
         var record = dbcl.find().sort({_id:-1}).limit(1);
         while(record.next())
         {
            var logicalID = record.current().toObj()._id
            if(logicalID !== firstLogicalID)
            {
               throw buildException("repeatedInsertAndPopLastRecord", null, "check logicalID", firstLogicalID, logicalID);
            }
         }
         dbcl.pop({LogicalID:logicalID,Direction:-1});
         
         var doc = new StringBuffer();
         doc.append(stringLength, string);
         var strings = doc.toString();
         dbcl.insert({a:strings}); 
      }
   }catch(e)
   {
      throw buildException("repeatedInsertAndPopLastRecord", e, null, null, e);
      
   }
}

main();



