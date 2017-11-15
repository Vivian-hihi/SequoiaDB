/************************************
*@Description:  创建Id索引更新统计信息  
*@author:      liuxiaoxuan
*@createdate:  2017.11.09
*@testlinkCase: seqDB-11395
**************************************/
function main()
{	
   var csName = COMMCSNAME + "11395";
   commDropCS( db, csName, true, "drop CS in the beginning" );
	
   var csOption = { PageSize: 4096 };
   commCreateCS( db, csName, false, "", csOption );
		
   //create cl	
   var clOption = { AutoIndexId : false };
   var clName = COMMCLNAME + "11395";
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true );
	
   //get master/slave datanode
   var db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "m"} );
   var dbclPrimary = db1.getCS(csName).getCL(clName);
   
   db1 = new Sdb(db);
   db1.setSessionAttr( {PreferedInstance: "s"} );
   var dbclSlave = db1.getCS(csName).getCL(clName);
	
   //insert
   var insertNums = 5000;
   insertDatas( dbcl, insertNums );
	
   //create id index
   createIdIndex( dbcl );
	
   //check before invoke analyze
   checkStat( db, csName, clName, "$id", false, false );
	
   //check the query explain of master/slave nodes 
   var findConf = {_id : 4000};
   var expExplains = [{ScanType:"ixscan", IndexName:"$id", ReturnNum:1}];
   
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );
	
   println("check result before analyze success!");
	                                                                    
   //invoke analyze
   var options = {CollectionSpace: csName};
   analyze( db, options );
                                                                                       
   //check after analyze
   checkStat( db, csName, clName, "$id", true, true );
 
   //check the query explain of master/slave nodes 
   var findConf = {_id : 4000};
   var expExplains = [{ScanType:"ixscan", IndexName:"$id", ReturnNum:1}];
	
   var actExplains = getCommonExplain( dbclPrimary, findConf);
   checkExplain( actExplains, expExplains );
   
   var actExplains = getCommonExplain( dbclSlave, findConf);
   checkExplain( actExplains, expExplains );

   println("check result after analyze success!");
   
   db1.close();
   commDropCS( db, csName, true, "drop CS in the end" );
}

function insertDatas( dbcl, insertNum )
{  
   try
   {
      var doc = [];
      for(var i = 0;i < insertNum;i++)
      {
         doc.push({_id:i, a:"test" + i});
      }
      dbcl.insert(doc);
   }
   catch(e)
   {
      throw buildException("insert datas", e, "insert", "insert success", e);
   }
}
function createIdIndex( dbcl )
{
   try
   {
      dbcl.createIdIndex();
   }
   catch(e)
   {
      throw buildException("create id index", e, "create IdIndex", "success", e);
   }
}
main();
