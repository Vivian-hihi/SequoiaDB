/************************************
*@Description：切分源组和目标组不在同一domain中 
*@author：2019-3-13 wangkexin
*@testlinkCase: seqDB-10506
**************************************/
var csName = COMMCSNAME + "_cs10506";
var clName = CHANGEDPREFIX + "_cl10506" ;
function main()
{
    var domainName = "mydomain10506";
    
    if( true == commIsStandalone( db ) )
    {
        println( "run mode is standalone" );
        return;
    }     
    
    //less three groups to split
    var allGroupName = getGroupName2(db,true);         
    if( 3 > allGroupName.length )
    {
        println("--least three groups");
        return ;
    }
    
    var groupsInfo = getGroupName2(db, true);
    var srcGrName_a = groupsInfo[0][0];
    var tarGrName_b = groupsInfo[1][0];
    var tarGrName_c = groupsInfo[2][0];
    
    db.createDomain( domainName, [ srcGrName_a, tarGrName_b ],{ AutoSplit:true } );
    println("autosplit srcGrName :" + srcGrName_a + " , tarGrName : " + tarGrName_b);
    commCreateCS(db, csName, false, "", {Domain:domainName});
    var options = {ShardingKey:{a:1},ShardingType:"hash",ReplSize:0};
    var cl = commCreateCLByOption(db, csName, clName, options, false);
    insertData( db, csName, clName, 100 );
    
    println("--start split");
    try
    {
        println("srcGrName :" + srcGrName_a + " , tarGrName : " + tarGrName_c);
        cl.split( srcGrName_a,  tarGrName_c, { Partition: 10 }, { Partition: 20 } )
        throw buildException("split()",null,"split should fail", "split failed", "split success");
    }
    catch( e )
    {
        if(e!=-216)
        {
            throw buildException("split()",e ,"target group is not in domain", '-216', e );
        }
    }
    
    println("--start check split result");
    //查看数据切分结果，分别连接coord、源组data、目标组data查询
    checkResult(COORDHOSTNAME, COORDSVCNAME, null, null, 100);
    checkResult(groupsInfo[0][1], groupsInfo[0][2], groupsInfo[1][1], groupsInfo[1][2], 100);
    
    //再次插入数据
    insertData( db, csName, clName, 100 );
    checkResult(COORDHOSTNAME, COORDSVCNAME, null, null, 200);
    checkResult(groupsInfo[0][1], groupsInfo[0][2], groupsInfo[1][1], groupsInfo[1][2], 200);
    
    println("--clean cs and domain");
    commDropCS( db, csName, true, "drop CS in the end" );
    db.dropDomain(domainName);
}

function checkResult( hostname1, svcname1, hostname2, svcname2, expCount )
{
    var db1 = new Sdb( hostname1, svcname1 );
    var actCount1 = db1.getCS(csName).getCL(clName).find().count();
    
    if(hostname2 == undefined && svcname2 == undefined)
    {
        if(actCount1!=expCount)
        {
            throw buildException("count()",e ,"The number of queries received by connecting "+hostname1+":"+svcname1+" is incorrect", expCount, actCount1 );
        }
        db1.close();
    }
    else
    {
        var db2 = new Sdb( hostname2, svcname2 );
        var actCount2 = db2.getCS(csName).getCL(clName).find().count();
        var actCount = actCount1+actCount2;
        if(actCount!=expCount)
        {
            throw buildException("count()",e ,"The number of queries received by connecting "+hostname1+":"+svcname1+" and "+hostname2+":"+svcname2+"  is incorrect actCount1 = " + actCount1 + " ,actCount2 = " + actCount2, expCount, actCount );
        }
        db1.close();
        db2.close();
    }
}
main();