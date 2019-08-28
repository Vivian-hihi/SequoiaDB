package com.sequoiadb.lob.randomwrite;

import java.util.Arrays;

import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.lob.utils.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @FileName seqDB-13257: read模式下锁定数据，读取lob 
 * @Author linsuqiang
 * @Date 2017-11-08
 * @Version 1.00
 */

/*
 * 1、打开lob，指定read模式 
 * 2、指定范围锁定数据段 
 * 3、读取lob（如seek偏移范围为锁定数据范围） 4、检查读lob结果
 */

public class LockAndRead13257 extends SdbTestBase {

    private final String csName = "writelob13257";
    private final String clName = "writelob13257";
    private final int lobPageSize = 4 * 1024; // 4k
    
    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    
    @BeforeClass
    public void setUp() {
    	sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");            
        // create cs cl
        BSONObject csOpt = (BSONObject) JSON.parse("{LobPageSize: " + lobPageSize + "}");
        cs = sdb.createCollectionSpace(csName, csOpt);
        BSONObject clOpt = (BSONObject) JSON.parse("{ShardingKey:{a:1},ShardingType:'hash'}");
        cl = cs.createCollection(clName, clOpt);        
    }

    @Test
    public void testLob() {
    	int lobSize = 300 * 1024;
        byte[] writeData = RandomWriteLobUtil.getRandomBytes(lobSize);
        ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, writeData);
            
        int offset = 50;
        int length = 100;
        byte[] readData = new byte[length]; 
        try (DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_READ)) {
            lob.lock(offset, length);
            lob.seek(offset, DBLob.SDB_LOB_SEEK_SET);
            lob.read(readData);
        }
            
        byte[] expData = Arrays.copyOfRange(writeData, offset, offset + length);
        RandomWriteLobUtil.assertByteArrayEqual(readData, expData, "lob data is wrong");        
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
}
