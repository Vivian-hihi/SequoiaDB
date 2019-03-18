/************************************
*@Description:  不同coord节点删除同一张子表
*@author:      wangkexin
*@createDate:  2019.3.12
*@testlinkCase: seqDB-10934
**************************************/
function main()
{
    var csName = COMMCSNAME + "_cs10934";
    var mainCL_Name = CHANGEDPREFIX + "_maincl10934" ;
    var subCL_Name = CHANGEDPREFIX + "_subcl10934";
    var clNum = 10;
    
    if(true == commIsStandalone( db ))
    {
      println( "run mode is standalone");
      return;
    }
    
    println("--test start.");
    try
    {
        var db1 = new Sdb( COORDHOSTNAME, COORDSVCNAME );
        
        //创建cs
        commCreateCS(db1, csName, true, "" );
        
        //创建主表
        var mainCLOption = { ShardingKey:{"a":1}, ShardingType:"range", IsMainCL:true};
        var maincl = commCreateCLByOption( db1, csName, mainCL_Name, mainCLOption, true, true);
        
        //创建子表
        var subClOption = {ShardingKey:{"b":1}, ShardingType:"hash", AutoSplit:true, ReplSize:0};
        commCreateCLByOption( db1, csName, subCL_Name, subClOption, true, true );
        
        //挂载子表
        var options = {LowBound:{ a:1 },UpBound:{ a: 100 }};
        maincl.attachCL(csName + "." + subCL_Name, options);

        //插入数据
        maincl.insert({a:10});
        
        //删除子表
        db1.getCS(csName).dropCL(subCL_Name);
        
        var db2 = new Sdb( COORDHOSTNAME, COORDSVCNAME );
        try
        {
            db2.getCS(csName).dropCL(subCL_Name);
            throw buildException("dropCL()",null,"drop collection should fail", "dropCL failed", "dropCL success");
        }
        catch( e )
        {
            if( e!=-23 )
            {
                throw buildException("dropCL()",e ,"drop collection should fail", '-23', e );
            }
        }
        
        //通过主表查询数据
        var cursor = db2.getCS(csName).getCL(mainCL_Name).find();
        if(cursor.next()!=null)
        {
            throw buildException("find",null,"check record", "no data", "have data");
        }
    }
    finally
    {
        if(db1 != null)
        {
            db1.close();
        }
        if(db2 != null)
        {
            db2.close();
        }
    }
    
    println("--test end.");
    //清除环境
    commDropCS( db, csName, true, "drop CS in the end" );  
}
main();