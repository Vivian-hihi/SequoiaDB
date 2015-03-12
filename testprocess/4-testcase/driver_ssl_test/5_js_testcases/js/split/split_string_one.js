
//*get the primary datagroup name ,return the primary datagroup name in string
function getPG( db, catadb )
{
   var GroupID = catadb.SYSCAT.SYSCOLLECTIONSPACES.find();

   for( var i = 0 ; i < GroupID.size() ; ++i )
   {
       var eachID = eval("("+GroupID[i]+")");
       if( CSPREFIX_CS == eachID["Name"] )
	   {
           GroupID = eachID["Group"][0]["GroupID"];
           break;
       }
   }

   var strCoord = db.listReplicaGroups();
   for( var i = 1 ; i != strCoord.size() ; ++i )
   {
       var estrCoord = eval('('+strCoord[i]+')');
       if(estrCoord["GroupID"] == GroupID )
	   {
           return (estrCoord["GroupName"]);
       }
   }
}

//*****get GroupName ,return array**************
//eg : arrGroupName[?][0] == "GroupName"
//               arrGroupName[?][1] == "GroupID"
function getGroupName( db )
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

    var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME );

	if ( commIsStandalone( db ) )
	{
	    println( "The usecase can't run in standalone" ) ;
		return 0 ;
	}

    try
	{
        db.dropCS( CSPREFIX_CS );
    }
	catch( e )
	{
	}

    try
	{
	    if( "" == db.listReplicaGroups() )
		{
	   	    return 0;
	    }
    }
	catch( e )
	{
	    if( e == SDB_RTN_COORD_ONLY )
		{
		    return 0 ;
        }
    }

    try
	{
        var claSize = new RSize( CSPREFIX_CS );
        //createCS
        var varCS = db.createCS(CSPREFIX_CS);

        //createCL , split in a
        var varCL = varCS.createCL(CSPREFIX_CL,{ShardingKey:{a:1} , ReplSize:claSize.ReplSize(), Compressed:true } );

    }
	catch( e )
	{
       throw e ;
    }

    var arrGroupName = getGroupName( db ) ;
    try
	{
        if( !(1 < arrGroupName.length) )
		{
            println("least two groups");
            db.dropCS( CSPREFIX_CS );
            throw e;
        }
    }
	catch( e )
	{
       return 0;
    }

    var catadb = new SecureSdb( COORDHOSTNAME, CATASVCNAME );
    var PGname = getPG( db, catadb ) ;
    var _PGname = PGname;

    var t = 1;

    //SLgroupID are groupsID
    var SLgroupID = [];

    for( var i = 0 ; i != arrGroupName.length ; ++i )
	{
        if( PGname == arrGroupName[i][0] )
		{
            SLgroupID.push( arrGroupName[i][1] );
            break;
        }
    }

    var str_in = ["a" , "b" , "c" , "d" , "e"];
    //groups split in sepecific condition
    if( 2 == arrGroupName.length )
    {
        for(var i=0; i !=arrGroupName.length ;++i)
		{
            if( PGname != arrGroupName[i][0] )
			{
                try
				{
                    varCL.split( PGname,arrGroupName[i][0],{a:"b"} );
                    SLgroupID.push(arrGroupName[i][1]);
                }
				catch(e)
				{
                    throw e;
                }
                break;
            }
        }
    }
	else
	{ 
	    //groups are more than two,we split three groups only
        for(var i=0; i != 3 && t < 5;++i)
		{
            if( PGname != arrGroupName[i][0] )
			{
                if( _PGname != arrGroupName[i][0] )
				{
                    try
					{
                        //println(PGname+"~~~~"+arrGroupName[i][0]);
                       varCL.split( PGname,arrGroupName[i][0],{a:str_in[t]} );
                       PGname = arrGroupName[i][0] ;
                       SLgroupID.push( arrGroupName[i][1] );
                       t = t+2 ;
                    }
					catch(e)
					{
                        throw e;
                    }
                }
            }
        }
    }

    var j = -2 ;
    //insert data
    for( var i = 0 ; i != 100*SLgroupID.length ; ++i )
	{
        if( 0 == i%100 )
		{
           j = j + 2;
        }
        
        try
		{
            varCL.insert({a:str_in[j]});
        }
		catch( e )
		{
            throw e;
        }
        //println( SLgroupID.length);
    }

    for(var i = 0 ; i != SLgroupID.length ; ++i)
	{
        var gdb = new SecureSdb( COORDHOSTNAME, (SLgroupID[i]-0));
        try
		{
            var len = eval( "gdb."+CSPREFIX_CS+"."+CSPREFIX_CL+".find().count()" );
        }
		catch(e)
		{
             throw e;
        }

        if(len != 100 )
		{
            throw -1;
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

