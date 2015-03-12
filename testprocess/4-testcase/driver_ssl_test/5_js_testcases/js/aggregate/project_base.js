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
   cl.insert({no:1000,major:"计算机科学与技术",dep:"计算机学院",info:{name:"Tom",age:25,sex:"男"}});
   cl.insert({no:1001,major:"计算机科学与技术",dep:"计算机学院",info:{name:"Json",age:20,sex:"男"}});
   cl.insert({no:1002,major:"计算机软件与理论",dep:"计算机学院",info:{name:"Holiday",age:22,sex:"女"}});
   cl.insert({no:1003,major:"计算机软件与理论",dep:"计算机学院",info:{name:"Sam",age:30,sex:"男"}});
   cl.insert({no:1004,major:"计算机工程",dep:"计算机学院",info:{name:"Coll",age:26,sex:"男"}});
   cl.insert({no:1006,major:"计算机工程",dep:"计算机学院",info:{name:"Jim",age:24,sex:"女"}});
   cl.insert({no:1007,major:"物理学",dep:"物电学院",info:{name:"Lily",age:28,sex:"女"}});
   cl.insert({no:1008,major:"物理学",dep:"物电学院",info:{name:"Lucy",age:36,sex:"女"}});
   cl.insert({no:1009,major:"光学",dep:"物电学院",info:{name:"Coco",age:27,sex:"女"}});
   cl.insert({no:1010,major:"光学",dep:"物电学院",info:{name:"Jack",age:30,sex:"男"}});
   cl.insert({no:1011,major:"电学",dep:"物电学院",info:{name:"Mike",age:28,sex:"男"}});
   cl.insert({no:1012,major:"电学",dep:"物电学院",info:{name:"Jaden",age:20,sex:"男"}});
}
catch (e)
{
	println("happend error when insert records,e="+e);
	throw e ;	
}

try
{
	var rc = cl.aggregate({$project:{no:1,major:1,dep:0,"info.name":1,"info.sex":0}});
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
	if(!((fieldarray.length == 3) && (fieldarray[0]=="no"&&fieldarray[1]=="major"&&fieldarray[2]=="info.name")))
	{
		println("return wrong field names");
		for(var j=0;j<fieldarray.length;j++)
		{
			println("fieldarray["+j+"]="+fieldarray[j]);	
		}
		throw -1 ;
	}
}

if( i != 12 )
{
	println("return wrong numbers of records");
	println(cl.aggregate({$project:{no:1,major:1,dep:0,"info.name":1,"info.sex":0}}));
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
