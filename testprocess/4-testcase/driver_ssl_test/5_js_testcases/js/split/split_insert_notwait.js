
//*get the primary datagroup name ,return the primary datagroup name in string
function getPG(){
var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);

var catadb = new SecureSdb(COORDHOSTNAME,CATASVCNAME);
/*
var cs = catadb.listCollections();

cs = cs.toArray();

cs = eval("("+cs[0]+")");

var _cs = cs["Name"];
*/


var GroupID = catadb.SYSCAT.SYSCOLLECTIONSPACES.find();

for( var i = 0 ; i < GroupID.size() ; ++i ){

var eachID = eval("("+GroupID[i]+")");

if( CSPREFIX_CS == eachID["Name"] ){

 GroupID = eachID["Group"][0]["GroupID"];
 break;

}

}

var strCoord = db.listReplicaGroups();

for(var i = 1 ; i != strCoord.size() ; ++i ){

    var estrCoord = eval('('+strCoord[i]+')');

    if(estrCoord["GroupID"] == GroupID ){
            return (estrCoord["GroupName"]);
    }
}
}

//*****get GroupName ,return array**************
//eg : arrGroupName[?][0] == "GroupName"
//               arrGroupName[?][1] == "GroupID"
function getGroupName(db)
{
	try
    {
    	var RGname = db.listReplicaGroups();
    }
    catch (e)
    {
    	throw e;
    }
    var j = 0;
    var arrGroupName = Array();
    for (var i=1 ; i != RGname.size() ; ++i )
    {
    	var eRGname = eval('('+RGname[i]+')') ;
    	arrGroupName[j] = Array();
    	arrGroupName[j].push(eRGname["GroupName"]) ;
      arrGroupName[j].push( eRGname["Group"][0]["Service"][0]["Name"] );
      ++j;

    }
    return arrGroupName;
}
function main()
{


CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;

var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);

try{
db.dropCS( CSPREFIX_CS );

}catch( e ){}
try{
if( "" == db.listReplicaGroups() ){
 	return 0;
}
}catch( e ){ if( e == SDB_RTN_COORD_ONLY ){ return 0 ; }}
try
{

var claSize = new RSize( CSPREFIX_CS );

//createCS
var varCS = db.createCS(CSPREFIX_CS);
}
catch(e)
{
	println("can't create CS");
	throw e;
}
println("createCS finished");
try
{
//createCL , split in a
//var varCL = varCS.createCL(CSPREFIX_CL,{ShardingKey:{id:1} , ReplSize:claSize.ReplSize(),Compressed:true});
var varCL = varCS.createCL(CSPREFIX_CL,{ShardingKey:{id:1} , ReplSize:0,Compressed:true});

}catch( e )
{
	println("can't create CL");
	throw e ;
}
println("create finished");
var arrGroupName = getGroupName(db);

try{
    if( !(2 < arrGroupName.length) ){
            println("least two groups");
            db.dropCS( CSPREFIX_CS );
            throw e;
    }

}catch( e ){
return 0;
}

var PGname = getPG();

var _PGname = PGname;

var t = 1;

var catadb = new SecureSdb(COORDHOSTNAME,CATASVCNAME);

//SLgroupID are groupsID
var SLgroupID = [];

for( var i = 0 ; i != arrGroupName.length ; ++i ){
    if( PGname == arrGroupName[i][0] ){
            SLgroupID.push( arrGroupName[i][1] );
            break;

    }
}
//insert data
for( var i = 0 ; i != 500*3 ; ++i )
{
	try{
		varCL.insert({id:i});
		}catch( e )
		{
			throw e;
    }
}

//groups are more than two,we split three groups only
for(var i=0; i != 3 && t < 3;++i)
{
	if( PGname != arrGroupName[i][0] )
	{
		if( _PGname != arrGroupName[i][0] )
		{
			println(PGname+"~~~~"+arrGroupName[i][0]);
			try
			{
				if(t<2)
				{
					varCL.split( _PGname,arrGroupName[i][0],{id:500*t},{id:500*(t+1)} );
				}
				else
				{
					varCL.split( _PGname,arrGroupName[i][0],{id:500*t});
				}
			}
			catch(e)
			{
				println(e);
				throw e;
			}
			//while( catadb.SYSCAT.SYSTASKS.find().count() != 0 );
			for(var j=500*t;j<500*(t+1);j++)//don't wait the split finished,insert data
			{
				try
				{
					varCL.insert({id:j});
				}
				catch(e)
				{
					println("insert err"+e);
					throw e;
				}
			}
			PGname = arrGroupName[i][0] ;
			SLgroupID.push( arrGroupName[i][1] );
			++t;
		}
	}
}

println("split and insert is end");
println("SLgroupID:"+SLgroupID);
for(var i = 0 ; i != SLgroupID.length ; ++i)
{
	var gdb = new SecureSdb(COORDHOSTNAME,(SLgroupID[i]-0));
	try
	{
		var array =gdb.getCS(CSPREFIX_CS).getCL(CSPREFIX_CL).find().toArray();
		println(array.length)
	}catch(e)
	{
		println(e);
	}
}
println(catadb.SYSCAT.SYSCOLLECTIONS.find());
var result_array=db.getCS(CSPREFIX_CS).getCL(CSPREFIX_CL).find().toArray();
println("result_array.length:"+result_array.length);
for(var i=0;i<10;i++)
{
	var index=Math.round(Math.random()*500*SLgroupID.length);
	
	println("index:"+index+"	"+result_array[index]);
}
println("result_array:"+result_array.length);
try
{
   db.dropCS( CSPREFIX_CS );

}catch( e )
{ 
	println(e);
}
}


try
{
   // Inspect the run mode is standalone or not
   if( true == commIsStandalone( db ) )
      throw "ModeStandAlone" ;
   main();
}
catch( e )
{
   if( "ModeStandAlone" == e )
      println( "The run mode is standalone" ) ;
   else
      throw e ;
}

