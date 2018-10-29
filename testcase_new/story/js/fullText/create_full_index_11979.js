/***************************************************************************
@Description :seqDB-11979 :集合属性不影响固定集合 
@Modify list :
              2018-10-26  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
	  return;
   };

   var clName = COMMCLNAME + "_ES_11979";
   commDropCL(db, COMMCSNAME, clName, true, true);
   
   var groups = commGetGroups( db );
   var arrayGroup = new Array();
   for (var i in groups){
      arrayGroup.push(groups[i][0]["GroupName"]);
   }
   
   if (arrayGroup.length <= 0){
      throw buildException("commGetGroups()", "commGetGroups", "can not get groups ", "success", "fail");
   }
   //指定集合的replSize、group、AutoIndexId、压缩为非默认值
   var dbcl = commCreateCLByOption( db, COMMCSNAME, clName, {ReplSize : 0, Group : arrayGroup[0], AutoIndexId : false, Compressed : true, CompressionType : "lzw"} );
   
   commCreateIndex( dbcl, "a", {content:"text"});
   commCheckIndex( dbcl, "a", true );
   
   //固定集合属性为默认值(与原集合属性无关)
   var dbOperator = new DBOperator();
   var cappedCLName = dbOperator.getCappedCLName( dbcl, "a" );
   var cappedDB = db.getRG(arrayGroup[0]).getMaster().connect();
   var cappedAttr = cappedDB.snapshot(4, {Name : cappedCLName + "." + cappedCLName});
   var cappedAttr = cappedAttr.next().toObj();
   if (cappedAttr["Details"][0]["Attribute"] != "NoIDIndex | Capped" || cappedAttr["Details"][0]["CompressionType"] != "" || cappedAttr["Details"][0]["Status"] != "Normal"){
      throw buildException("snapshot()", "snapshot", "the capped cl is not default value ", "success", "fail");
   }
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}
main()