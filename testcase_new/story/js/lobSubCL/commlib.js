/************************************
*@Description: 创建主表，默认创建两个字表
*@author:      luweikang
*@createDate:  2019.8.7
**************************************/
function createMainCLAndAttachCL(db, csName, mainCLName, clName, shardingFormat, subCLNum, beginBound, scope)
{
    if ( shardingFormat == undefined ) { shardingFormat = "YYYYMMDD" ; }
    if ( subCLNum == undefined ) { subCLNum = 2 ; }
    if ( beginBound == undefined ) 
    {
        switch( shardingFormat )
        {
            case "YYYYMMDD": 
                beginBound = new Date().getFullYear() * 10000 + 101;
                break;
            case "YYYYMM":
                beginBound = new Date().getFullYear() * 100 + 1;
                break;
            case "YYYY":
                beginBound = new Date().getFullYear();
                break;
            default:
                beginBound = new Date().getFullYear() * 10000 + 101;
        }
    }
    else
    {
        var dateArr = beginBound.toString().split('');
        switch( shardingFormat )
        {
            case "YYYYMMDD": 
                beginBound = parseInt(beginBound.toString(), 10);
                break;
            case "YYYYMM":
                beginBound = parseInt(dateArr[0] + dateArr[1] + dateArr[2] + dateArr[3] + dateArr[4] + dateArr[5], 10);
                break;
            case "YYYY":
                beginBound = parseInt(dateArr[0] + dateArr[1] + dateArr[2] + dateArr[3], 10);
                break;
            default:
                beginBound = new Date().getFullYear() * 10000 + 101;
        }
    }
    
    if ( scope == undefined ) { scope = 5 ; }
    
    var options = {"IsMainCL": true, "ShardingKey": {"date": 1}, "LobShardingKeyFormat": shardingFormat, "ShardingType": "range"};
    var mainCL = commCreateCLByOption(db, csName, mainCLName, options, true, false, "create main cl");
    
    for(var i = 0; i < subCLNum; i++)
    {
        var subCLName = clName + "_" + i;
        commCreateCLByOption(db, csName, subCLName, {"ShardingKey": {"date": 1}, "ShardingType":"hash", "AutoSplit": true});
        var lowBound = {"date": ( parseInt(beginBound) + i * scope ) + ''};
        var upBound = {"date": ( parseInt(beginBound) + (i + 1) * scope ) + ''};
        mainCL.attachCL( csName + "." + subCLName, {"LowBound": lowBound, "UpBound": upBound});
    }
    
    return mainCL;
}

/************************************
*@Description: 构造临时文件用于插入lob
*@author:      luweikang
*@createDate:  2019.8.7
**************************************/
function makeTmpFile( filePath, fileName, fileSize)
{
    println("---create tmp file---");
    if( fileName == undefined){ println( "---error msg: fileName is null." );}
    if( fileSize == undefined){ fileSize = 1024 * 100; }    
    var fileFullPath = filePath + "/" + fileName;        
	File.mkdir(filePath);
    
	var cmd = new Cmd();
    cmd.run( "dd if=/dev/zero of=" + fileFullPath  + " bs=1c count=" + fileSize );
    var md5Arr = cmd.run( "md5sum " + fileFullPath ).split(" ");
    var md5 = md5Arr[0];
    return md5
}

/************************************
*@Description: 删除临时文件
*@author:      luweikang
*@createDate:  2019.8.7
**************************************/
function deleteTmpFile( filePath )
{
    try
    {
        File.remove( filePath );
    }
    catch(e)
    {
        if(e != -4 )
        {
            throw e;
        }
    }
}

/************************************
*@Description: 插入lob到主表中，lobNum是每个子表会插入的lob数，默认每个子表插入10个lob
*@author:      luweikang
*@createDate:  2019.8.7
**************************************/
function insertLob(mainCL, filePath, format, scope, lobNum, subCLNum, beginDate)
{
    println("---put lob---");
    if( lobNum == undefined ){ lobNum = 10; }
    if( subCLNum == undefined ){ subCLNum = 2; }
    if( scope == undefined ){ scope = 5; }
    if( format == undefined){ format = "YYYYMMDD" }
    
    var lobOids = [];
    if( beginDate != undefined)
    {
        var dateArr = beginDate.toString().split('');
        var year = parseInt(dateArr[0] + dateArr[1] + dateArr[2] + dateArr[3], 10);
        var month = parseInt(dateArr[4] + dateArr[5], 10);
        var day = parseInt(dateArr[6] + dateArr[7], 10);
    }
    else
    {
        var year = new Date().getFullYear();
        var month = 1;
        var day = 1;
    }
    
    for( var i = 0; i < subCLNum; i++)
    {
        for(var j = 0; j < lobNum; j++)
        {
            var timestamp = null;
            switch(format)
            {
                case "YYYY":
                    timestamp = (year + i * scope) + "-" + month + "-" + day + "-00.00.00.000000";
                    break;
                case "YYYYMM":
                    timestamp = year + "-" + (month + i * scope) + "-" + day + "-00.00.00.000000";
                    break;
                case "YYYYMMDD":
                    timestamp = year + "-" + month + "-" + (day + i * scope) + "-00.00.00.000000";
                    break;
                default:
                    timestamp = year + "-" + month + "-" + (day + i * scope) + "-00.00.00.000000";
            }
            var lobOid = mainCL.createLobID( timestamp );
            try
            {
                lobOids[i * lobNum + j] = mainCL.putLob( filePath, lobOid );
            }
            catch( e )
            {
                println("---error msg: filePath: " + filePath + ", timestamp: " + timestamp);
                throw e;
            }
            
        }
        
    }
    return lobOids;
}

/************************************
*@Description: 读取lob并获取md5
*@author:      luweikang
*@createDate:  2019.8.7
**************************************/
function readLobs(mainCL, lobOids)
{
    var clName = mainCL.toString().split(".")[2];
    var lobFileMd5s = [];
    for(var i=0; i< lobOids.length; i++)
    {
        var cmd = new Cmd();
        var lobReadPath = WORKDIR + "/lob_" + clName + "_" + i;
        try
        {
            File.remove( lobReadPath );
        }
        catch(e){}
        mainCL.getLob( lobOids[i], lobReadPath);
        var md5Arr = cmd.run( "md5sum " + lobReadPath ).split(" ");
        var md5 = md5Arr[0];
        lobFileMd5s.push( md5 );
        File.remove(lobReadPath);
    }
    
    return lobFileMd5s;
}

/************************************
*@Description: 通过比较MD5检查插入的lob内容是否正确
*@author:      luweikang
*@createDate:  2019.8.7
**************************************/
function checkLobMD5(mainCL, lobOids, fileMD5)
{
    println("---check lob md5---");
    var lobFileMd5s = readLobs(mainCL, lobOids);
    for(var i = 0; i < lobOids.length; i++)
    {
        if(lobFileMd5s[i] != fileMD5)
        {
            throw buildException( "checkLobMD5", null, "compare the lob to MD5 of the file, LobOID: " + lobOids[i], fileMD5, lobFileMd5s[i] );
        }
    }
}

/************************************
*@Description: 获取主表下的所有子表
*@author:      luweikang
*@createDate:  2019.8.7
**************************************/
function getSubCLNames(db, mainCLFullName)
{
    var clFullNames = [];
    var cur = db.snapshot(8, {Name: mainCLFullName});
    if(cur.next())
    {
        var clInfo = cur.current();
        var cataInfo = clInfo.toObj().CataInfo
        for( i in cataInfo)
        {
            clFullNames.push(cataInfo[i].SubCLName);
        }
    }
    return clFullNames;
}

/************************************
*@Description: 检查主表lob落到子表区域
*@author:      luweikang
*@createDate:  2019.8.7
**************************************/
function checkSubCLLob(db, mainCLFullName, lobOids)
{
    println("---check sub cl lob---");
    var subCLNames = getSubCLNames(db, mainCLFullName);
    var clName = mainCLFullName.split(".")[1];
    var lobNum = lobOids.length / subCLNames.length;
    for(i in subCLNames)
    {
        var nameArr = subCLNames[i].split(".");
        var subCL = db.getCS(nameArr[0]).getCL(nameArr[1]);
        var actlobNum = 0;
        var cur = subCL.listLobs();
        var tmpLobIds = [];
        while(cur.next())
        {
            actlobNum++;
            tmpLobIds.push(cur.current());
        }
        if(actlobNum != lobNum)
        {
            throw buildException( "checkSubCLLob()", null, "check subCL lob num: " + subCLNames[i] + ", lobOid: " + tmpLobIds, lobNum, actlobNum );
        }
        for(var j = 0; j < lobNum; j++)
        {
            var lobReadPath = WORKDIR + "/sublob_" + clName + "_" + (i * lobNum + j);
            try
            {
                File.remove(lobReadPath);
            }catch(e)
            {
                if( e != -4 )
                {
                    throw e;
                }
            }
            subCL.getLob(lobOids[i * lobNum + j], lobReadPath);
            File.remove(lobReadPath);
        }
    }
}

/************************************
*@Description: 删除主表
*@author:      luweikang
*@createDate:  2019.8.7
**************************************/
function cleanMainCL(db, mainCSName, mainCLName)
{
    println("---clean main cl---");
    var subCLNames = getSubCLNames(db, mainCSName + "." + mainCLName);
    commDropCL(db, mainCSName, mainCLName);
    for(i in subCLNames)
    {
        var name = subCLNames[i].split(".");
        var csName = name[0];
        var clName = name[1];
        try
        {
            db.getCS(csName).getCL(clName);
            throw buildException( "cleanMainCL()", null, "get not exist cl: " + subCLNames[i], "not exist", "exist" );
        }
        catch( e )
        {
            if( e !== -23 )
            {
                throw e;
            }
        }
    }
}

/************************************
*@Description: 带匹配条件listLobs，然后比较listLobs结果
*@author:      wuyan
*@createDate:  2019.8.21
**************************************/
function listLobsAndCheckResult(mainCL, condition, attrName, attrValue, matchSymbol)
{
    println("---begin to listLob use " + matchSymbol);
    var listResult = mainCL.listLobs(SdbQueryOption().sort({"Oid":1}));
    var expListResult = [];
    while( listResult.next() )
    {
        var listObj = listResult.current().toObj();
        var objValue = listObj[attrName];
        switch(matchSymbol)
        {
            case "$lt":                 
                if ( objValue < attrValue )
                {
                    expListResult.push(listObj);
                }
                break;
            case "$lte":
                if ( objValue <= attrValue )
                {
                    expListResult.push(listObj);
                }
                break;
             case "$gt":
                if ( objValue > attrValue )
                {
                    expListResult.push(listObj);
                }
                break;
            case "$gte":
                if ( objValue >= attrValue )
                {
                    expListResult.push(listObj);
                }
                break;
            case "$ne":
                if ( objValue != attrValue )
                {
                    expListResult.push(listObj);
                }
                break;
            case "$et":            
                if ( objValue == attrValue )
                {                   
                    expListResult.push(listObj);
                }
                break;            
            default:
                break;
        }    
    }
   
    var actRecs = [];
    var rc = mainCL.listLobs(SdbQueryOption().cond( condition ).sort({"Oid":1}));   
    while( rc.next() )
    {
		actRecs.push( rc.current().toObj() );
    } 
    
    println("---begin to check result.");
    if( JSON.stringify(actRecs) !== JSON.stringify(expListResult))
    {
        println("\nactual value= "+JSON.stringify(actRecs)+"\nexpect value= "+JSON.stringify(expListResult)); 
        throw buildException("checkRec()", "rec ERROR,the list condition=" + JSON.stringify(condition));
    }
}

/************************************
*@Description: 获取cl切分的目标组名
*@author:      wuyan
*@createDate:  2019.8.21
**************************************/
function getTargetGroup( csName, clName, srcGroupName )
{
    if( undefined == csName || undefined == clName )
    {
        println( "cs name: " + csName + ", clName: " + clName ) ;
        throw "cs or cl name is undefined" ;
    }
    var tableName = csName + "." + clName ;
    var allGroupInfo = commGetGroups(db);  
    var targetGroupName;
    for( var i = 0 ; i < allGroupInfo.length ; ++i ) 
    {
        var groupName = allGroupInfo[i][0].GroupName;
        if( srcGroupName != groupName )
        {            
            targetGroupName = groupName;           
            break;				
        }
    }    
    return targetGroupName;   
}

















