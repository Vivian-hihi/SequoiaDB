/************************************
*@Description: 指定domain创建子表，设置分区数小于domain所包含组个数 
*@author:      wangkexin
*@createDate:  2019.3.11
*@testlinkCase: seqDB-15669
**************************************/
main();
function main()
{
    var csName = "cs15669" ;
    var mainCL_Name = "maincl15669" ;
    var subCL_Name = "subcl15669";
    var domainName = "domain15669";
    
    if(true == commIsStandalone( db ))
    {
      println( "run mode is standalone.");
      return;
    }
    
    //新创建6个组
    var groupsArray = commGetGroups( db, false, "", true, true, true );
    var dataRGNum = groupsArray.length;
    if(dataRGNum < 3)
    {
      println( "at least three data groups.");
      return;
    }
    var hostName = groupsArray[1][1].HostName;
    var newDataRGNames = createDataGroups( db, hostName, 6 );

    //将新建的6个组和环境中原有的3个（或3个以上）的组名一起存放在totalDataRGNames中
    var totalDataRGNames = getDateRGNames( newDataRGNames, groupsArray );
    
    db.createDomain( domainName, totalDataRGNames, {AutoSplit:true} );
    commCreateCS( db, csName, true, "", {Domain:domainName} );
    
    createMainCL( csName, mainCL_Name );
    println("start to create sub cl.");
    var subClOption = {ShardingKey:{"b":1}, ShardingType:"hash", AutoSplit:true, Partition:8, ReplSize:0};
    try
    {
        db.getCS( csName ).createCL( subCL_Name, subClOption );
        throw buildException( "createCL()", null, "create collection should fail", "createCL failed", "createCL success" );
    }catch( e )
    {
        if( e!==-6 )
        {
            throw buildException( "main()", e, "create subCL", '-6', e );
        }
    }
    
    //清除环境
    commDropCS( db, csName, true, "drop CS in the end" );
    removeDataRG( db, newDataRGNames );
    db.dropDomain( domainName );  
}

function createMainCL( csName, mainCL_Name )
{
    println( "start to create main cl." );
    var mainCLOption = { ShardingKey:{"a":1}, ShardingType:"range", IsMainCL:true };
    var dbcl = commCreateCLByOption( db, csName, mainCL_Name, mainCLOption, true, true );
}

function createDataGroups( db, hostName , groupNum )
{
    var dataGroupNames = [];
    for( var i = 0; i < groupNum; i++ )
    {
        var port = parseInt( RSRVPORTBEGIN )+( i*10 );
        var rgName = "group15669_" + i;
        var dataRG = db.createRG( rgName );
        dataRG.createNode( hostName, port, RSRVNODEDIR+"data/"+port );
        dataRG.start();
        dataGroupNames.push( rgName );
    }
    return dataGroupNames;
}

function getDateRGNames( newDataRGNames, originalDataRG )
{
    var DataRGNames = [];
    for(var i=0 ; i < newDataRGNames.length ; i++)
    {
        DataRGNames.push( newDataRGNames[i] );
    }
    
    for(var i = 0 ; i < originalDataRG.length; i++)
    {
        DataRGNames.push( originalDataRG[i][0].GroupName );
    }
    
    return DataRGNames;
}

function removeDataRG( db, dataGroupNames )
{
    for(var i = 0 ; i < dataGroupNames.length; i++)
    {
        db.removeRG( dataGroupNames[i] );
    }
}