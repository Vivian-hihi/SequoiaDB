/***************************************************************************
@Description :seqDB-11978 :集合空间属性不影响固定集合  
@Modify list :
              2018-10-26  YinZhen  Create
****************************************************************************/
function main()
{
   if(commIsStandalone( db )){
      println("Deploy is standalone");
      return;
   };

   var clName = COMMCLNAME + "_ES_11978";
   var csName = "testCS_ES_11978";
   commDropCS( db, csName );
   
   //指定集合空间的PageSize、LobPageSize指定为非默认值
   commCreateCS( db, csName, false, {PageSize : 4096, LobPageSize : 4096} );
   var dbcl = commCreateCL( db, csName, clName );
   
   commCreateIndex( dbcl, "a", {content:"text"});
   commCheckIndex( dbcl, "a", true );
   
   //固定集合属性为默认值(与原集合属性无关)
   var dbOperator = new DBOperator();
   var cappedCLName = dbOperator.getCappedCLName( dbcl, "a" );
   var group = commGetCLGroups( db, csName + "." + clName );
   
   var cappedDB = db.getRG(group[0]).getMaster().connect();
   var cappedAttr = cappedDB.snapshot(4, {Name : cappedCLName + "." + cappedCLName});
   var cappedAttr = cappedAttr.next().toObj();
   if (cappedAttr["Details"][0]["PageSize"] != 65536 || cappedAttr["Details"][0]["LobPageSize"] != 262144){
      throw buildException("main()", "capped cl's attributes is not default value", "equal to PageSize and LobPageSize", 
	     "PageSize : 65536, LobPageSize : 262144", "PageSize : " + cappedAttr["Details"][0]["PageSize"] + " LobPageSize " + cappedAttr["Details"][0]["LobPageSize"]);
   }
   
   commDropCS( db, csName );
}

main()