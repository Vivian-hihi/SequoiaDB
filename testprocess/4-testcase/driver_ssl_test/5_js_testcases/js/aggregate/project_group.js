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

try
{
   cl.insert({no:1000,interest:["basketball","football"],major:"计算机科学与技术",dep:"计算机学院",info:{name:"Tom",age:25,sex:"男"}});	
   cl.insert({no:1001,major:"计算机科学与技术",dep:"计算机学院",info:{name:"Json",age:20,sex:"男"}});

   cl.insert({no:1002,interest:["movie","photo"],major:"计算机软件与理论",dep:"计算机学院",info:{name:"Holiday",age:22,sex:"女"}});
   cl.insert({no:1003,major:"计算机软件与理论",dep:"计算机学院",info:{name:"Sam",age:30,sex:"男"}});

   cl.insert({no:1004,interest:["basketball","football","movie"],major:"计算机工程",dep:"计算机学院",info:{name:"Coll",age:26,sex:"男"}});
   cl.insert({no:1006,major:"计算机工程",dep:"计算机学院",info:{name:"Jim",age:24,sex:"女"}});

   cl.insert({no:1007,interest:["basketball","football","movie","photo"],major:"物理学",dep:"物电学院",info:{name:"Lily",age:28,sex:"女"}});
   cl.insert({no:1014,interest:["basketball","football","photo"],major:"物理学",dep:"物电学院",info:{name:"Kiki",age:18,sex:"女"}});
   cl.insert({no:1015,interest:["basketball","football","movie"],major:"物理学",dep:"物电学院",info:{name:"Appie",age:20,sex:"女"}});
   cl.insert({no:1008,major:"物理学",dep:"物电学院",info:{name:"Lucy",age:36,sex:"女"}});

   cl.insert({no:1009,major:"光学",dep:"物电学院",info:{name:"Coco",age:27,sex:"女"}});
   cl.insert({no:1010,major:"光学",dep:"物电学院",info:{name:"Jack",age:30,sex:"男"}});
   cl.insert({no:1011,interest:["basketball","movie"],major:"光学",dep:"物电学院",info:{name:"Mike",age:28,sex:"男"}});

   cl.insert({no:1013,interest:["basketball","movie","photo"],major:"电学",dep:"物电学院",info:{name:"Jaden",age:20,sex:"男"}});
   cl.insert({no:1016,interest:["football","movie","photo"],major:"电学",dep:"物电学院",info:{name:"Iccra",age:19,sex:"男"}});
   cl.insert({no:1017,major:"电学",dep:"物电学院",info:{name:"Jay",age:15,sex:"男"}});
   cl.insert({no:1018,major:"电学",dep:"物电学院",info:{name:"Kate",age:20,sex:"男"}});
}
catch (e)
{
	println("happend error when insert records,e="+e);
	throw e ;	
}
try
{
	var rc = cl.aggregate({$match:{interest:{$exists:1}}},{$project:{major:1,"info.age":1}},{$group:{_id:"$major",avg_age:{$avg:"$info.age"}}},{$sort:{avg_age:-1}});
}
catch(e)
{
	println("failed to execut aggregate function,e="+e);
	throw e ;	
}

var i =0 ;
while(rc.next())
{
	i++;
	var fieldarray =  new Array();
	var recordJson = rc.current().toJson();
	var obj = eval("("+recordJson+")");
	for(var fieldname in obj)
	{
		fieldarray.push(fieldname);
	}
	if(!((fieldarray.length == 1) && (fieldarray[0]=="avg_age")))
	{
		println("return wrong field names");
		for(var j=0;j<fieldarray.length;j++)
		{
			println("fieldarray["+j+"]="+fieldarray[j]);	
		}
		throw -1 ;
	}
}

if( i != 6 )
{
	println("return wrong numbers of records");
	println(cl.aggregate({$match:{interest:{$exists:1}}},{$project:{major:1,"info.age":1}},{$group:{_id:"$major",avg_age:{$avg:"$info.age"}}},{$sort:{avg_age:-1}}));
	throw -1 ;
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

