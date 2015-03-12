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
	var rc = cl.aggregate({$group:{_id:{dep:"$dep"},Major:{$first:"$major"},first_score:{$first:"$score"},last_score:{$last:"$score"}}});
//	println(rc) ;
}
catch(e)
{
	println("failed to execut aggregate function group,e="+e);
	throw e ;	
}

var i =0 ;
var firstscore = new Array();
var lastscore = new Array();
while(rc.next()){
	i++ ;
	firstscore.push(rc.current().toObj()["first_score"]);
	lastscore.push(rc.current().toObj()["last_score"]);
	var recordJson = rc.current().toJson();
	var obj = eval("("+recordJson+")");
	var fieldarray = new Array();
	for(var fieldname in obj)
	{
		fieldarray.push(fieldname);
	}	
	
	if(!((fieldarray.length == 3) && (fieldarray[0]=="Major"&&fieldarray[1]=="first_score"&&fieldarray[2]=="last_score")))
	{
		println("return wrong field names");
		for(var j=0;j<fieldarray.length;j++)
		{
			println("fieldarray["+j+"]="+fieldarray[j]);	
		}
		throw -1 ;
	}
}
var firstScore = new Array(72,80);
//println(firstscore.length) ;
for(var k=0;k<firstscore.length;k++)
{
	if(firstscore[k]!=firstScore[k])
	{
		println("the information of first_score field is wrong,firstscore["+k+"]="+firstscore[k]);
		throw -1 ;	
	}	
}
var lastScore = new Array(93,82);
for(var k=0;k<lastscore.length;k++)
{
	if(lastscore[k]!=lastScore[k])
	{
		println("the information of last_score field is wrong,lastscore["+k+"]="+lastscore[k]);
		throw -1 ;	
	}	
}

if( i != 2 )
{
	println(rc) ;
	println("return wrong numbers of records,i="+i);
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
