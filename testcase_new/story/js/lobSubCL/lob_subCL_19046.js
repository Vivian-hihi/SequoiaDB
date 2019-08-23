/************************************
*@Description: seqDB-19046 主表挂载已切分的hash子表
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
    var groups = commGetGroups(db);
    if ( groups.length < 2)
    {
        println("--least two groups");
        return ;
    }
    
    var csName = COMMCSNAME;
    var mainCLName = "mainCL_19046";
    var subCLName = "subCL_19046";
    var sourceGroup = groups[0][0].GroupName;
    var targetGroup = groups[1][0].GroupName;
    var filePath = WORKDIR + "/lob19046/";
    var fileName = "file19046"
    var fileFullPath = filePath + fileName;
    var fileMD5 = makeTmpFile( filePath, fileName );
    
    commDropCL(db, csName, mainCLName);
    commDropCL(db, csName, subCLName);
    
    var options = {"IsMainCL": true, "ShardingKey": {"date": 1}, "LobShardingKeyFormat": "YYYYMMDD", "ShardingType": "range"};
    var mainCL = commCreateCLByOption(db, csName, mainCLName, options, true, false, "create main cl");
    var clOptions = {"ShardingKey": {"a": 1}, ShardingType:"hash", Group: sourceGroup};
    var subCL = commCreateCLByOption(db, csName, subCLName, clOptions, true, false, "create sub cl");

    var lobOids1 = insertLob(subCL, fileFullPath, "YYYYMMDD", 5, 10, 1, "20190801");
    subCL.split(sourceGroup, targetGroup, 50);
    
    mainCL.attachCL( csName + "." + subCLName, {"LowBound": {"date": "20190801"}, "UpBound": {"date": "20190831"}});
    
    var lobOids2 = insertLob(mainCL, fileFullPath, "YYYYMMDD", 5, 10, 1, "20190810");
    checkLobMD5(mainCL, lobOids1, fileMD5);
    checkLobMD5(mainCL, lobOids2, fileMD5);
    
    for(i in lobOids1)
    {
        mainCL.deleteLob(lobOids1[i]);
        try
        {
            mainCL.getLob(lobOids1[i], filePath + "/checkLob19046_" + i );
            throw 0;
        }
        catch( e )
        {
            if( e !== -4 )
            {
                throw buildException( "check delete lob", e, "gets the deleted lob: " + lobOids1[i], -4, e ); 
            }
        }
    }
    
    deleteTmpFile( filePath );
    commDropCL(db, csName, mainCLName);
    commDropCL(db, csName, subCLName);
}

main();