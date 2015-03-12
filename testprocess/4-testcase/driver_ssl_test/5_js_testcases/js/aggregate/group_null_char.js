/************************************************************************
*@Description: deal with "null" and "charactor" field type by using group
*              "avg/sum/min/max"
*@Modify list:
*              2014/3/10   huxiaojun
************************************************************************/
var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);
try
{
   // Drop collection in the beginning
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "failed to clean environment in the beginning" ) ;
   // Create Collection and auto specify CollectionSpaces
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, -1, true, true,
                          false, "Failed to create collection" ) ;
}
catch( e )
{
   throw e ;
}

//insert data to collection
try
{
   cl.insert({no:1000,score:80,interest:["basketball","football"],major:"计算机科学与技术",dep:"计算机学院",info:{name:"Tom",age:25,sex:"男"}});	
   cl.insert({no:1001,score:82,major:"计算机科学与技术",dep:"计算机学院",info:{name:"Json",age:null,sex:"男"}});

   cl.insert({no:1002,score:85,interest:["movie","photo"],major:"计算机软件与理论",dep:"计算机学院",info:{name:"Holiday",age:22,sex:"女"}});
   cl.insert({no:1003,score:90,major:"计算机软件与理论",dep:"计算机学院",info:{name:"Sam",age:30,sex:"男"}});

   cl.insert({no:1004,score:69,interest:["basketball","football","movie"],major:"计算机工程",dep:"计算机学院",info:{name:"Coll",age:null,sex:"男"}});
   cl.insert({no:1005,score:70,major:"计算机工程",dep:"计算机学院",info:{name:"Jim",age:24,sex:"女"}});

   cl.insert({no:1006,score:84,interest:["basketball","football","movie","photo"],major:"物理学",dep:"物电学院",info:{name:"Lily",age:null,sex:"女"}});
   cl.insert({no:1007,score:73,interest:["basketball","football","photo"],major:"物理学",dep:"物电学院",info:{name:"Kiki",age:"eighteen",sex:"女"}});
   cl.insert({no:1008,score:72,interest:["basketball","football","movie"],major:"物理学",dep:"物电学院",info:{name:"Appie",age:20,sex:"女"}});
   cl.insert({no:1009,score:80,major:"物理学",dep:"物电学院",info:{name:"Lucy",age:36,sex:"女"}});

   cl.insert({no:1010,score:93,major:"光学",dep:"物电学院",info:{name:"Coco",age:"twenty",sex:"女"}});
   cl.insert({no:1011,score:75,major:"光学",dep:"物电学院",info:{name:"Jack",age:"",sex:"男"}});
   cl.insert({no:1012,score:78,interest:["basketball","movie"],major:"光学",dep:"物电学院",info:{name:"Mike",age:28,sex:"男"}});

   cl.insert({no:1013,score:86,interest:["basketball","movie","photo"],major:"电学",dep:"物电学院",info:{name:"Jaden",age:20,sex:"男"}});
   cl.insert({no:1014,score:74,interest:["football","movie","photo"],major:"电学",dep:"物电学院",info:{name:"Iccra",age:19,sex:"男"}});
   cl.insert({no:1015,score:81,major:"电学",dep:"物电学院",info:{name:"Jay",age:15,sex:"男"}});
   cl.insert({no:1016,score:92,major:"电学",dep:"物电学院",info:{name:"Kate",age:"a",sex:"男"}});
}
catch (e)
{
   println("Error,Failed to insert data, e="+e);
   throw e ;
}

//aggregate function group[max/min/avg/sum]
try
{
   var rc = cl.aggregate({$group:{max_age:{$max:"$info.age"},min_age:{$min:"$info.age"},avg_age:{$avg:"$info.age"},sum_age:{$sum:"$info.age"}}});

}
catch(e)
{
   println("Error,Failed to excute aggregate function[group/max/min/avg/sum], e="+e);
   throw e ;
}

var max=rc.current().toObj()["max_age"] ;
var min=rc.current().toObj()["min_age"] ;
var avg=rc.current().toObj()["avg_age"] ;
var sum=rc.current().toObj()["sum_age"] ;

//Check over the field values
if("twenty"!=max || "15"!=min || "23.9"!=avg || "239"!=sum)
{
   println("rc.current().toJson()="+rc.current().toJson()) ;
   throw "Error,Wrong field values get!" ;
}

try
{
   // Clear environment in the end
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "failed to clean environment in the end" ) ;
}
catch (e)
{
   throw e ;
}
