package com.sequoiadb.lob.randomwrite;


import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.lob.utils.RandomWriteLobUtil;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @FileName seqDB-13259: 切分表加锁写lob 
 * @Author linsuqiang
 * @Date 2017-11-08
 * @Version 1.00
 */

/*
 * 1、写入lob数据 
 * 2、打开已存在lob对象 
 * 3、指定偏移范围锁定数据段（lockAndSeek），其中锁定范围覆盖目标组和源组范围 
 * 4、写入lob 
 * 5、读取lob，检查操作结果
 */

public class LockAndWriteSplitCL13259 extends SdbTestBase {

    private final String csName = "writelob13259";
    private final String clName = "writelob13259";
    private final int lobPageSize = 16 * 1024; // 16k
    
    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    
    @BeforeClass
    public void setUp() {
    	sdb = new Sequoiadb(SdbTestBase.coordUrl, "", ""); 
        if (CommLib.isStandAlone(sdb) || CommLib.OneGroupMode(sdb)) {
        	throw new SkipException("no groups to split");
        }
            
        // create cs cl
        BSONObject csOpt = (BSONObject) JSON.parse("{LobPageSize: " + lobPageSize + "}");
        cs = sdb.createCollectionSpace(csName, csOpt);
        BSONObject clOpt = (BSONObject) JSON.parse("{ShardingKey:{a:1},ShardingType:'hash'}");
        cl = cs.createCollection(clName, clOpt);
            
        // split cl 
        String srcGroupName = RandomWriteLobUtil.getSrcGroupName(sdb, csName, clName);
        String dstGroupName = RandomWriteLobUtil.getSplitGroupName(sdb, srcGroupName);
        cl.split(srcGroupName, dstGroupName, 50);       
    }

    @Test
    public void testLob() { 
    	int lobSize = 300 * 1024;
        byte[] data = RandomWriteLobUtil.getRandomBytes(lobSize);
        ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, data);
        byte[] expData = data;
            
        LobPart partA = new LobPart(0         , 100 * 1024);
        LobPart partB = new LobPart(120 * 1024, 80 * 1024);
        LobPart partC = new LobPart(210 * 1024, 50 * 1024);            
        
        try( DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)) {                
            lockAndSeekAndWriteLob(lob, partA);
            lockAndSeekAndWriteLob(lob, partB);
            lockAndSeekAndWriteLob(lob, partC);                
            expData = updateExpData(expData, partA);
            expData = updateExpData(expData, partB);
            expData = updateExpData(expData, partC);
        } 
        byte[] actData = RandomWriteLobUtil.readLob(cl, oid);
        RandomWriteLobUtil.assertByteArrayEqual(actData, expData, "lob data is wrong");          
    }

    @AfterClass
    public void tearDown() {
        try {
            if (sdb.isCollectionSpaceExist(csName)) {
                sdb.dropCollectionSpace(csName);
            }
        } finally {
            if (null != sdb) {
                sdb.close();
            }
        }
    }
    
    private void lockAndSeekAndWriteLob(DBLob lob, LobPart part) {
        lob.lockAndSeek(part.getOffset(), part.getLength());
        lob.write(part.getData());
    }
    
    private byte[] updateExpData(byte[] expData, LobPart part) {
        return RandomWriteLobUtil.appendBuff(expData, part.getData(), part.getOffset());
    }

}
