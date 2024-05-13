/******************************************************************************
 * @Description   : seqDB-33948:部分节点开启消息压缩，如1主1备，构造重复率高及低的消息
 * @Author        : huangxiaoni
 * @CreateTime    : 2023.03.01
 * @LastEditTime  : 2024.05.11
 * @LastEditors   : huangxiaoni
                    wenjingwang
 ******************************************************************************/
testConf.skipStandAlone = true;
testConf.csName = COMMCSNAME + "_33948";
testConf.clName = "cl_shard_33948";
testConf.clOpt = { "ShardingKey": { "a": 1 }, "AutoSplit": true, "ReplSize": -1 }


main( setUp, test );

function updateConfOfGroup(rg)
{
    var nodenames = [];
    // 主节点
    var nodeName = rg.getMaster().toString();
    nodenames.push(nodeName) 
    
    // 备节点
    var nodeName = rg.getSlave().toString();
    nodenames.push(nodeName) 
    db.updateConf( { "netcompressor": "lz4" }, { "NodeName": nodenames } );
}

function setUp(testpara)
{
   var groupNames = commGetDataGroupNames( db );
   // 数据组1主1备开启消息压缩
   for( var i = 0; i < groupNames.length; i++ )
   {
      var rg = db.getRG( groupNames[i] );
      updateConfOfGroup(rg);
   }
   // 编目组1主1备开启消息压缩
   var rg = db.getCataRG();
   updateConfOfGroup(rg);
} 

function test (testpara)
{
   // 初始化：关闭消息压缩
   db.updateConf( { "netcompressor": "" } );
   try
   {

    // 写重复率高的数据
      var docs = insertHighDuplicateDocs(testpara.testCL);
      findAndcheckResult(testpara.testCL, docs);
      
      testpara.testCL.truncate();
    
    // 写重复率低的数据
      docs = insertLowDuplicateDocs(testpara.testCL);
      findAndcheckResult(testpara.testCL, docs);
   
   }
   finally
   {
      db.updateConf( { "netcompressor": "" } );
   }
}
