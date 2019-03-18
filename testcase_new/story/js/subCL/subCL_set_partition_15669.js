/************************************
*@Description: 指定domain创建子表，设置分区数小于domain所包含组个数 
*@author:      wangkexin
*@createDate:  2019.3.11
*@testlinkCase: seqDB-15669
**************************************/
function main()
{
    var csName = CHANGEDPREFIX + "_cs15669" ;
    var mainCL_Name = CHANGEDPREFIX + "_maincl15669" ;
    var subCL_Name = CHANGEDPREFIX + "_subcl15669";
    var domainName = "domain15669";
    
    if(true == commIsStandalone( db ))
    {
      println( "run mode is standalone");
      return;
    }
    
    //创建多个组
    var groupsArray = commGetGroups(db, false, "", false, true, true );
    var hostName = groupsArray[1][1].HostName;
    var dataGroupNames = createDataGroups(db, hostName, 10);
    
    println("start create domain.");
    //创建Domain
    db.createDomain(domainName,dataGroupNames,{AutoSplit:true});
    //创建cs
    commCreateCS(db, csName, true, "", {Domain:domainName});
    //创建主表
    var mainCLOption = { ShardingKey:{"a":1}, ShardingType:"range", IsMainCL:true};
    var dbcl = commCreateCLByOption( db, csName, mainCL_Name, mainCLOption, true, true);
    
    //创建子表
    var subClOption = {ShardingKey:{"b":1}, ShardingType:"hash", AutoSplit:true, Partition:8, ReplSize:0};
    try
    {
        db.getCS(csName).createCL(subCL_Name, subClOption);
        throw buildException("createCL()",null,"create collection should fail", "createCL failed", "createCL success");
    }catch( e )
    {
        if( e!=-6 )
        {
            throw buildException("createCL()",e ,"create collection should fail", '-6', e );
        }
    }
    
    println("--test end.");
    //清除环境
    commDropCS( db, csName, true, "drop CS in the end" );
    removeDataRG(db, dataGroupNames);
    db.dropDomain(domainName);  
}

function createDataGroups( db, hostName , groupNum )
{
    var dataGroupNames = [];
    for(var i = 0; i < groupNum; i++)
    {
        var port = parseInt(RSRVPORTBEGIN)+(i*10);
        var rgName = "group15669_" + i;
        var dataRG = db.createRG(rgName);
        dataRG.createNode(hostName, port, RSRVNODEDIR+"data/"+port);
        dataRG.start();
        dataGroupNames.push( rgName );
    }
    return dataGroupNames;
}

function removeDataRG( db, dataGroupNames )
{
    for(var i = 0 ; i < dataGroupNames.length; i++)
    {
        db.removeRG(dataGroupNames[i]);
    }
}
main();