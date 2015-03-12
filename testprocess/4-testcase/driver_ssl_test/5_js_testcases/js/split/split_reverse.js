
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
try{

var claSize = new RSize( CSPREFIX_CS );

//createCS
var varCS = db.createCS(CSPREFIX_CS);

//createCL , split in a
var varCL = varCS.createCL(CSPREFIX_CL,{ShardingKey:{id:1} , ReplSize:claSize.ReplSize(),Compressed:true});

}catch( e ){
 throw e ;
}

var arrGroupName = getGroupName(db);

try{
      if( !(1 < arrGroupName.length) ){
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

//groups split in sepecific condition
if( 2 == arrGroupName.length )
{
//insert data
for(var i=0;i<500*2;i++)
{
	try
	{
		varCL.insert({id:i});
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

}



for(var i = 0 ; i != SLgroupID.length ; ++i){
      var gdb = new SecureSdb(COORDHOSTNAME,(SLgroupID[i]-0));
      //var gdb = new SecureSdb(COORDHOSTNAME,60000);

      try{
              var len = eval( "gdb."+CSPREFIX_CS+"."+CSPREFIX_CL+".find().size()" );
              println(len)
      }catch(e){
           throw e;
      }
      if(500!=len)
      {
      	println("len is less than expected");
      }
      
}

PGname=_PGname;
t=1;
//SLgroupID are groupsID
var SLgroupID = [];

for( var i = 0 ; i != arrGroupName.length ; ++i ){
      if( PGname == arrGroupName[i][0] ){
              SLgroupID.push( arrGroupName[i][1] );
              break;

      }
}

//groups recover
if( 2 == arrGroupName.length )
{
      for(var i=0; i !=arrGroupName.length ;++i){

              if( PGname != arrGroupName[i][0] ){

                      try{
                         varCL.split( arrGroupName[i][0],PGname,{id:500} );
                         while( catadb.SYSCAT.SYSTASKS.find().count() != 0 );
                         SLgroupID.push(arrGroupName[i][1]);
                      }catch(e){
                              throw e;
                      }
                      break;
              }
      }

}else{ //groups are more than two,we split three groups only
      for(var i=0; i != 3 && t < 3;++i){

              if( PGname != arrGroupName[i][0] ){

                      if( _PGname != arrGroupName[i][0] ){

                              try{
                                      println(arrGroupName[i][0]+"~~~~"+_PGname);


                                      varCL.split( arrGroupName[i][0],_PGname,{id:500*t} );
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

}
println(SLgroupID);
for(var i = 0 ; i != SLgroupID.length ; ++i){
      var gdb = new SecureSdb(COORDHOSTNAME,(SLgroupID[i]-0));
      //var gdb = new SecureSdb(COORDHOSTNAME,60000);

      try{
              var len = eval( "gdb."+CSPREFIX_CS+"."+CSPREFIX_CL+".find().count()" );
              println(len)
      }catch(e){
      	if(-23!=e)
      	{
      		throw e;
      	}
      	else
      	{
      		break;
      	}
      }
      
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

