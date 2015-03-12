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
   cl.insert({no:1000,score:80,interest:["basketball","football"],major:"计算机科学与技术",dep:"计算机学院",info:{name:"Tom",age:25,sex:"男"}});	
   cl.insert({no:1001,score:82,major:"计算机科学与技术",dep:"计算机学院",info:{name:"Json",age:20,sex:"男"}});

   cl.insert({no:1002,score:85,interest:["movie","photo"],major:"计算机软件与理论",dep:"计算机学院",info:{name:"Holiday",age:22,sex:"女"}});
   cl.insert({no:1003,score:90,major:"计算机软件与理论",dep:"计算机学院",info:{name:"Sam",age:30,sex:"男"}});

   cl.insert({no:1004,score:69,interest:["basketball","football","movie"],major:"计算机工程",dep:"计算机学院",info:{name:"Coll",age:26,sex:"男"}});
   cl.insert({no:1005,score:70,major:"计算机工程",dep:"计算机学院",info:{name:"Jim",age:24,sex:"女"}});

   cl.insert({no:1006,score:84,interest:["basketball","football","movie","photo"],major:"物理学",dep:"物电学院",info:{name:"Lily",age:28,sex:"女"}});
   cl.insert({no:1007,score:73,interest:["basketball","football","photo"],major:"物理学",dep:"物电学院",info:{name:"Kiki",age:18,sex:"女"}});
   cl.insert({no:1008,score:72,interest:["basketball","football","movie"],major:"物理学",dep:"物电学院",info:{name:"Appie",age:20,sex:"女"}});
   cl.insert({no:1009,score:80,major:"物理学",dep:"物电学院",info:{name:"Lucy",age:36,sex:"女"}});

   cl.insert({no:1010,score:93,major:"光学",dep:"物电学院",info:{name:"Coco",age:27,sex:"女"}});
   cl.insert({no:1011,score:75,major:"光学",dep:"物电学院",info:{name:"Jack",age:30,sex:"男"}});
   cl.insert({no:1012,score:78,interest:["basketball","movie"],major:"光学",dep:"物电学院",info:{name:"Mike",age:28,sex:"男"}});

   cl.insert({no:1013,score:86,interest:["basketball","movie","photo"],major:"电学",dep:"物电学院",info:{name:"Jaden",age:20,sex:"男"}});
   cl.insert({no:1014,score:74,interest:["football","movie","photo"],major:"电学",dep:"物电学院",info:{name:"Iccra",age:19,sex:"男"}});
   cl.insert({no:1015,score:81,major:"电学",dep:"物电学院",info:{name:"Jay",age:15,sex:"男"}});
   cl.insert({no:1016,score:92,major:"电学",dep:"物电学院",info:{name:"Kate",age:20,sex:"男"}});
}
catch (e)
{
   println("happend error when insert records,e="+e);
   throw e ;
}
try
{
	var rc = cl.aggregate({$match:{interest:{$exists:1}}},{$group:{_id:"$major",avg_age:{$avg:"$info.age"},major:{$first:"$major"}}},{$sort:{avg_age:-1,major:-1}},{$skip:2},{$limit:3});

}
catch(e)
{
	println("failed to execut aggregate function group,e="+e);
	throw e ;	
}
var i =0 ;
var age = new Array();
var Major = new Array();
while(rc.next()){
	i++ ;
	var recordJson = rc.current().toJson();
	var obj = eval("("+recordJson+")");
	//验证字段名是否只有major和avg_score字段
	var fieldarray = new Array();

	age.push(obj["avg_age"]);
	
	Major.push(obj["major"]);
	for(var fieldname in obj)
	{
		fieldarray.push(fieldname);
	}	
	if(!((fieldarray.length == 2) && (fieldarray[0]=="avg_age"&&fieldarray[1]=="major")))
	{
		println("return wrong field names");
		for(var j=0;j<fieldarray.length;j++)
		{
			println("fieldarray["+j+"]="+fieldarray[j]);	
		}
		throw -1 ;
	}
}

if(!((Major[0]=="计算机科学与技术"&&Major[1]=="计算机软件与理论"&&Major[2]=="物理学")&&(age[0]=="25"&&age[1]=="22"&&age[2]=="22")))
{
	println("return wrong information,..");
	println(cl.aggregate({$match:{interest:{$exists:1}}},{$group:{_id:"$major",avg_age:{$avg:"$info.age"},major:{$first:"$major"}}},{$sort:{avg_age:-1,major:-1}},{$skip:2},{$limit:3}))	;
	throw -1 ;
}
if( i != 3 )
{
   println("return wrong numbers of records,i="+i);
   println(cl.aggregate({$match:{no:{$gt:1001},"info.sex":"男"}},{$group:{_id:"$major",major:{$first:"$major"},avg_score:{$first:"$score"}}},{$match:{avg_score:{$gte:70}}},{$sort:{avg_score:-1,major:1}},{$skip:1},{$limit:1}));
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
