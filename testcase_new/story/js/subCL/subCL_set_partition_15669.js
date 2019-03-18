/************************************
*@Description: 指定domain创建子表，设置分区数小于domain所包含组个数 
*@author:      wangkexin
*@createDate:  2019.3.11
*@testlinkCase: seqDB-15669
**************************************/
function main()//TODO: 用例有涉及2.8分支版本，请合入
{
    var csName = CHANGEDPREFIX + "_cs15669" ;//TODO: "cs15669"，不需要加CHANGEDPREFIX，这个是之前不好的代码习惯，不用参考
    var mainCL_Name = CHANGEDPREFIX + "_maincl15669" ;//TODO: 同上
    var subCL_Name = CHANGEDPREFIX + "_subcl15669";//TODO: 同上
    var domainName = "domain15669";
    
    if(true == commIsStandalone( db ))
    {
      println( "run mode is standalone");
      return;
    }
    
    //创建多个组
    var groupsArray = commGetGroups(db, false, "", false, true, true );
    var hostName = groupsArray[1][1].HostName;
    var dataGroupNames = createDataGroups(db, hostName, 10);//TODO: 只在3组3节点测，新增6个组，每组一个节点
    
    println("start create domain.");//TODO: 打印信息很奇怪，测试点没有打印信息创建domain有，建议此处删除，在创建主子表打印
    //创建Domain   //TODO: 方法名很明显的可以不用写批注
    db.createDomain(domainName,dataGroupNames,{AutoSplit:true});
    //创建cs      //TODO：同上，批注去掉
    commCreateCS(db, csName, true, "", {Domain:domainName});
    //创建主表    //TODO: 创建主子表放到一个另一个方法里面，main里面代码尽量简洁
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
        if( e!=-6 )//TODO: 比较值都用3个等号，"==="或"!=="
        {
            throw buildException("createCL()",e ,"create collection should fail", '-6', e );
            /* TODO: 括号/逗号前后空格统一，逗号后需要有空格，另外变量前后空格
               TODO: buildException( 函数名（此处为main，在其他方法的话则使用实际对应的方法名）,...,
                     操作（如create subCL）, ... )，上面信息跟方法对不上 */
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
main();   //TODO: 写到最前面