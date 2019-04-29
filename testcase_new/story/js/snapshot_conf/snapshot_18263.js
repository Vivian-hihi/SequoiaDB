/***************************************************************************
@Description : seqDB-15956:指定快照查询参数Mode为Run查询配置快照信息
@Modify list :
              2019-4-17  wangkexin  Create
****************************************************************************/
main();
function main()
{
   if(commIsStandalone( db ))
   {
      println("Deploy is standalone");
      return;
   }
   var groups = commGetGroups(db);
   var hostName = groups[0][1].HostName;
   var svcname = groups[0][1].svcname;
   var nodeName = hostName + ":" + svcname;
   var option1 = new SdbSnapshotOption().cond( { NodeName:nodeName } ).options( { "mode": "run", "expand": false } );
   checkResult( option1, true );
   
   try
   {
       try
       {
        db.deleteConf( { transactionon: 1 }, { 'NodeName': nodeName } );
       }
       catch( e )
       {
           if( e !== -264 )
               throw "unexpected error : " + e;
       }   
       var option2 = new SdbSnapshotOption().cond( { NodeName:nodeName } ).options( { "mode": "run", "expand": false } );
       checkResult( option2, true );
       
       var option3 = new SdbSnapshotOption().cond( { NodeName:nodeName } ).options( { "mode": "local", "expand": false } );
       checkResult( option3, false );
   }
   finally
   {
       //还原节点配置文件配置
       db.updateConf( { transactionon: true }, { 'NodeName': nodeName } );
   }
}

function checkResult( option, ifConfExist )
{
    var cursor = db.snapshot( SDB_SNAP_CONFIGS, option );
    var size=0;
    while( cursor.next() )
    {
        size++;
        var ret = cursor.current();
        if( ifConfExist )
        {
            if (ret.toObj().transactionon !== "TRUE") 
            {
                throw buildException("checkResult", "", "check value of configure transactionon", "TRUE", ret.toString());
            }
        }
        else
        {
            if (ret.toObj().transactionon !== undefined) 
            {
                throw buildException("checkResult", "", "transactionon should not exist", "not exist", ret.toString());
            }
        }
    }
    if( size !== 1 )
        throw "number error";
}