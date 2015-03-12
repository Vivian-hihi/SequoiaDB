
//*get the primary datagroup name ,return the primary datagroup name in string
function getPG(CS_NAME){
var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);

var catadb = new SecureSdb(COORDHOSTNAME,CATASVCNAME);
var GroupID = catadb.SYSCAT.SYSCOLLECTIONSPACES.find();

for( var i = 0 ; i < GroupID.size() ; ++i ){

  var eachID = eval("("+GroupID[i]+")");
  //println("eachID:");
  //println(eachID);
  if( CS_NAME == eachID["Name"] ){

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
   db.dropCS( CSPREFIX_CS+"2" );

}catch( e ){ println(e);}
try{
	if( "" == db.listReplicaGroups() ){
	   	return 0;
	}
}catch( e ){ if( e == SDB_RTN_COORD_ONLY ){ return 0 ; }}
try{

var claSize = new RSize( CSPREFIX_CS );
var claSize2 = new RSize( CSPREFIX_CS+"2" );

//createCS
var varCS = db.createCS(CSPREFIX_CS);
var varCS2 = db.createCS("foo2",8192);

var varCL = varCS.createCL(CSPREFIX_CL,{ShardingKey:{id:1} , ReplSize:claSize.ReplSize(),Compressed:true});
var varCL2 = varCS2.createCL(CSPREFIX_CL,{ShardingKey:{id:1} , ReplSize:claSize2.ReplSize(),Compressed:true});

}catch( e ){
	 println("can't create");
   throw e ;
}

var arrGroupName = getGroupName(db);

try{
        if( !(1 < arrGroupName.length) ){
                println("least two groups");
                db.dropCS( CSPREFIX_CS );
                db.dropCS( CSPREFIX_CS+"2" );
                throw e;
        }

}catch( e ){
   return 0;
}

try
{
println("start get PGname");
var PGname = getPG("foo");
println("end get PGname");
}
catch(e)
{
	println("can't get PGname");
}
println("PGname:"+PGname);

var _PGname = PGname;

try
{
println("start get PGname2");
var PGname2 = getPG("foo2");
println("end get PGname2");
}
catch(e)
{
	println("can't get PGname");
}
println("PGname2:"+PGname2);

var _PGname2 = PGname2;

var t = 1;

var catadb = new SecureSdb(COORDHOSTNAME,CATASVCNAME);

//SLgroupID are groupsID
var SLgroupID = [];
var SLgroupID2 = [];

for( var i = 0 ; i != arrGroupName.length ; ++i ){
        if( PGname == arrGroupName[i][0] ){
                SLgroupID.push( arrGroupName[i][1] );
                break;

        }
}
for( var i = 0 ; i != arrGroupName.length ; ++i ){
        if( PGname2 == arrGroupName[i][0] ){
                SLgroupID2.push( arrGroupName[i][1] );
                break;

        }
}

//groups split in sepecific condition
if( 2 == arrGroupName.length )
{
	//insert data
	for(var i=0;i<500*2;i++)
	{
		try
		{
			varCL.insert({id:i});
			varCL2.insert({id:i});
		}
		catch(e)
		{
			throw e;
		}
	}
        for(var i=0; i !=arrGroupName.length ;++i){

                if( PGname != arrGroupName[i][0] ){

                        try{
                           varCL.split( PGname,arrGroupName[i][0],{id:500} );
                           varCL2.split( PGname,arrGroupName[i][0],{id:500} );
                           while( catadb.SYSCAT.SYSTASKS.find().count() != 0 );
                           SLgroupID.push(arrGroupName[i][1]);
                        }catch(e){
                                throw e;
                        }
                        break;
                }
        }

}else{ //groups are more than two,we split three groups only
	//insert data
	for(var i=0;i<500*3;i++)
	{
		try
		{
			varCL.insert({id:i});
			varCL2.insert({id:i});
		}
		catch(e)
		{
			throw e;
		}
	}
        for(var i=0; i != 3 && t < 3;++i){

                if( PGname != arrGroupName[i][0] ){

                        if( _PGname != arrGroupName[i][0] ){

                                try{
                                        println(PGname+"~~~~"+arrGroupName[i][0]);
                                        varCL.split( PGname,arrGroupName[i][0],{id:500*t} );
                                        while( catadb.SYSCAT.SYSTASKS.find().count() != 0 );

                                        PGname = arrGroupName[i][0] ;

                                        SLgroupID.push( arrGroupName[i][1] );

                                        ++t;

                                 }catch(e){
                                        throw e;
                                 }

                        }

                }

        }
        t=1;//recover
        for(var i=0; i != 3 && t < 3;++i){

                if( PGname2 != arrGroupName[i][0] ){

                        if( _PGname2 != arrGroupName[i][0] ){

                                try{
                                        println(PGname2+"~~~~"+arrGroupName[i][0]);
                                        varCL2.split( PGname2,arrGroupName[i][0],{id:500*t} );
                                        while( catadb.SYSCAT.SYSTASKS.find().count() != 0 );

                                        PGname2 = arrGroupName[i][0] ;

                                        SLgroupID2.push( arrGroupName[i][1] );

                                        ++t;

                                 }catch(e){
                                        throw e;
                                 }

                        }

                }

        }

}


println("SLgroupID:"+SLgroupID);
for(var i = 0 ; i != SLgroupID.length ; ++i){
        var gdb = new SecureSdb(COORDHOSTNAME,(SLgroupID[i]-0));

        try{
                var len = eval( "gdb."+CSPREFIX_CS+"."+CSPREFIX_CL+".find().size()" );
                println(len);
        }catch(e){
             throw e;
        }
        
        if(len != 500){
                throw -1;
        }

}
println("SLgroupID2:"+SLgroupID2);
println("len2:");
for(var i = 0 ; i != SLgroupID2.length ; ++i){
        var gdb = new SecureSdb(COORDHOSTNAME,(SLgroupID2[i]-0));

        try{
                var len2 = eval( "gdb."+CSPREFIX_CS+"2"+"."+CSPREFIX_CL+".find().size()" );
                println(len2);
        }catch(e){
             throw e;
        }
        
        if(len2!=500 ){
                throw -1;
        }

}
try{
   db.dropCS( CSPREFIX_CS );
   db.dropCS( CSPREFIX_CS+"2" );

}catch( e ){ println(e);}
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

