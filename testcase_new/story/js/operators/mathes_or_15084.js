/* *****************************************************************************
@discretion: operators basic: $or
@modify list:
                     2018-09-18 csq  Init
***************************************************************************** */

try{
   var db = new Sdb(COORDHOSTNAME, COORDSVCNAME) ;
}catch(e)
{
   println("can't connect to db");
   throw e;
}

if (commGetGroupsNum(db)>=2)
{
   try{
      commDropCL( db, COMMCSNAME+"15084", COMMCLNAME+"15084", true, true,
                  "drop cl in the beginning" ) ;
   }catch( e ){}

   //create CL
   try{
      var groups = commGetGroups(db);
      var srcGroupName = groups[0][0].GroupName;
      var destGroupName = groups[1][0].GroupName;
      var varCS = commCreateCS( db, COMMCSNAME+"15084", true, "create CS" );
      var varCL = varCS.createCL(COMMCLNAME+"15084",{ShardingKey:{a:1},ShardingType:"hash",Group:srcGroupName});
      println("create CL finished");
   }catch(e)
   {
      println("can't create CL:" + COMMCLNAME+"15084" + " rc="+e);
      throw e;

   }

   //insert data
   try{
      varCL.split(srcGroupName,destGroupName,50);
      varCL.insert({a:0, b:1, c:0});
      varCL.insert({a:1,b:0,c:0});
   }catch(e)
   {
      println("insert data fail! rc="+e);
      throw e;
   }
   println("insert data finished");

   //no index
   //select * from ... where a=1 or b=10;
   try{
      var rc = varCL.find({"$or":[{"a":1},{"b":1}]}).toArray();
      if (rc.length !== 2)
      {
         throw "find result not ok";
      }
   }catch(e)
   {
      throw e;
   }
}



