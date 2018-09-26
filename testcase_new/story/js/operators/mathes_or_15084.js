/* *****************************************************************************
@discretion: operators basic: $or
@modify list:
                     2018-09-18 csq  Init
***************************************************************************** */

function main()
{
   try{
      var db = new Sdb(COORDHOSTNAME, COORDSVCNAME) ;
   }catch(e)
   {
      throw buildException("connect db fail", e, "connect", "success", e);
   }
   if (commIsStandalone(db))
   {
      println("skip standalone environment");
      return ;
   }
                                               	
   if (2 > commGetGroupsNum(db))
   {
      println("group less than 2");
      return ;
   }
   
   try{
      commDropCL( db, COMMCSNAME+"15084", COMMCLNAME+"15084", true, true,
                  "drop cl in the beginning" ) ;
   }catch( e ){}

   //create CL
   var groups = commGetGroups(db);
   var srcGroupName = groups[0][0].GroupName;
   var destGroupName = groups[1][0].GroupName;
   var varCS = commCreateCS( db, COMMCSNAME+"15084", true, "create CS" );
   var varCL = varCS.createCL(COMMCLNAME+"15084",{ShardingKey:{a:1},ShardingType:"hash",Group:srcGroupName});

   //insert data
   try{
      varCL.split(srcGroupName,destGroupName,50);
      varCL.insert({a:0, b:1, c:0});
      varCL.insert({a:1,b:0,c:0});
   }catch(e)
   {
      throw buildException("insert datas fail", e, "insert", "success", e);
   }

   //no index
   //select * from ... where a=1 or b=10;
   try{
      var rc = varCL.find({"$or":[{"a":1},{"b":1}]}).toArray();
      var expFindResult = [{a:0, b:1, c:0},{a:1,b:0,c:0}];
      heckRec(cur, expFindResult);
   }catch(e)
   {
      throw buildException("find datas fail", e, "find", "success", e);
   }
}

main();




