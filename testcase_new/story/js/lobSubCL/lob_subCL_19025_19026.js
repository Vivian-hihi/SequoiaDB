/************************************
*@Description:  seqDB-19025 创建主表，指定不同格式LobShardingKeyFirmat
                seqDB-19026 删除主表集合，主表已挂载子表 
*@author:      luweikang
*@createDate:  2019.8.12
**************************************/
function main()
{
    if(commIsStandalone( db ))
    {
        println("skip standalone mode");
        return;
    }
    var csName = COMMCSNAME;
    var mainCLName = "cl19025_main";
    var subCLName = "cl19025_sub";
    var filePath = WORKDIR + "/testfile19025";
    var fileMD5 = makeTmpFile( filePath );
    
    //测试创建LobShardingKeyFormat为YYYYMMDD主表
    var mainCL = createMainTable( db, csName, mainCLName, subCLName, "YYYYMMDD");
    var nameArr = mainCL.toString().split(".");
    var mainCLFullName = nameArr[1] + "." + nameArr[2] ;
    var lobOid1s = insertLob( mainCL, filePath, "YYYYMMDD" );
    checkLobMD5( mainCL, lobOid1s, fileMD5 );
    checkSubCLLob( db, mainCLFullName, lobOid1s);
    cleanMainCL(db, csName, mainCLName);
    
    //测试创建LobShardingKeyFormat为YYYYMM主表
    createMainTable( db, csName, mainCLName, subCLName, "YYYYMM");
    var lobOid2s = insertLob( mainCL, filePath, "YYYYMM" );
    checkLobMD5( mainCL, lobOid2s, fileMD5 );
    checkSubCLLob( db, mainCLFullName, lobOid2s);
    cleanMainCL(db, csName, mainCLName);
    
    //测试创建LobShardingKeyFormat为YYYY主表
    createMainTable( db, csName, mainCLName, subCLName, "YYYY");
    var lobOid3s = insertLob( mainCL, filePath, "YYYY" );
    checkLobMD5( mainCL, lobOid3s, fileMD5 );
    checkSubCLLob( db, mainCLFullName, lobOid3s);
    cleanMainCL(db, csName, mainCLName);
    
    deleteTmpFile( filePath );
}

main();