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

   cl.insert({no:1004,score:80,interest:["basketball","football","movie"],major:"计算机工程",dep:"计算机学院",info:{name:"Coll",age:26,sex:"男"}});
   cl.insert({no:1005,score:90,major:"计算机工程",dep:"计算机学院",info:{name:"Jim",age:24,sex:"女"}});

   cl.insert({no:1006,score:82,interest:["basketball","football","movie","photo"],major:"物理学",dep:"物电学院",info:{name:"Lily",age:28,sex:"女"}});
   cl.insert({no:1007,score:93,interest:["basketball","football","photo"],major:"物理学",dep:"物电学院",info:{name:"Kiki",age:18,sex:"女"}});
   cl.insert({no:1008,score:75,interest:["basketball","football","movie"],major:"物理学",dep:"物电学院",info:{name:"Appie",age:20,sex:"女"}});
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

//one field 
try
{
	var rc = cl.aggregate({$sort:{no:1}});
}
catch(e)
{
	println("failed to execut aggregate function sort one field,e="+e);
	throw e ;	
}

var i=0 ;
var No_array= new Array() ;	
while(rc.next())
{
	i++;
	var recordJson = rc.current().toJson() ;
	No_array.push(rc.current().toObj()["no"]) ;	
	var fieldarray =  new Array();
	var obj = eval("("+recordJson+")");
        for(var fieldname in obj)
        {
                fieldarray.push(fieldname);
        }	
}

for ( k=0;k<i;k++ )
{
	if(No_array[k+1]<No_array[k])
	{
		println("return wrong record information , noarray["+k+"]="+No_array[k]) ;
		throw -1 ;	
	}
}

if ( 17!=i )
{
	println("return wrong numbers of records") ;	
	println(cl.aggregate({$sort:{no:1}})) ;
	throw -1 ;		
}

//more field
try
{
	var rc = cl.aggregate({$sort:{score:-1,"info.age":1}});

}
catch(e)
{
	println("failed to execut aggregate function sort more field,e="+e);
	throw e ;	
}

var i=0 ;	
var more_array = new Array() ;
var more_array1 = new Array() ;
while(rc.next())
{
	i++ ;	
	var fieldarray = new Array() ;
	more_array.push(rc.current().toObj()["info.name"]) ;	
	more_array1.push(rc.current().toObj()["score"]) ;		
}

for (var k=0;k<i;k++)
{
	if((more_array[k+1]<=more_array[k]) && (more_array1[k+1]>more_array1[k]))
	{
		println("return wrong information["+k+"]") ;	
		throw -1 ;	
	}
}
if ( 17!=i )
{
	println("return wrong numbers of more sort records ") ;	
	println({$sort:{score:-1,"info.age":1}}) ;
	throw -1 ;		
}


//nest field
try
{
	var rc = cl.aggregate({$sort:{no:1,score:-1,"info.age":1}});

}
catch(e)
{
	println("failed to execut aggregate function sort nest field,e="+e);
	throw e ;	
}

//combine with $project $match $limit $skip $group $sort
try
{
	cl.aggregate({$sort:{no:1,score:-1,"info.age":1}},{$project:{major:1}},{$match:{$and:[{no:{$gt:1010}},{dep:"物电学院"}]}},{$limit:4},{$skip:1},{$group:{_id:"$major",avg_score:{$avg:"$score"},Major:{$first:"$major"}}}) ;	
}
catch ( e )
{
	println("Failed to execut combine function sort,project,e="+e) ;	
	throw e ;
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
