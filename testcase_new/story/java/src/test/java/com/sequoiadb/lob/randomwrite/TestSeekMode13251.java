package com.sequoiadb.lob.randomwrite;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.lob.utils.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;
import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.Arrays;

/**
 * @FileName seqDB-13251: 不同模式下执行seek操作 
 * @Author linsuqiang
 * @Date 2017-11-08
 * @Version 1.00
 */

/*
 * 1、打开lob对象，分别指定不同模式下执行seek操作： createonly、read、write 
 * 2、检查操作结果
 */

public class TestSeekMode13251 extends SdbTestBase {

    private final String csName = "writelob13251";
    private final String clName = "writelob13251";
    private final int lobPageSize = 128 * 1024; // 128k
    
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
        int offset = 10;
        byte[] expData = new byte[offset];
            
        // createonly mode
        int lobSize = 16 * 1024;
        ObjectId oid = null;
        try (DBLob lob = cl.createLob()) {
            lob.write(new byte[offset]); // fill zero
            lob.seek(offset, DBLob.SDB_LOB_SEEK_SET);
            byte[] data = RandomWriteLobUtil.getRandomBytes(lobSize);
            lob.write(data);
            expData = RandomWriteLobUtil.appendBuff(expData, data, offset);
            oid = lob.getID();
         }
            
         byte[] actData = RandomWriteLobUtil.readLob(cl, oid);
         RandomWriteLobUtil.assertByteArrayEqual(actData, expData, "lob data is wrong");
            
         // write mode
         try (DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)) {
        	 lob.seek(offset, DBLob.SDB_LOB_SEEK_SET);
        	 byte[] data = new byte[lobSize];
             lob.write(data);
             expData = RandomWriteLobUtil.appendBuff(expData, data, offset);
         }
         actData = RandomWriteLobUtil.readLob(cl, oid);
         RandomWriteLobUtil.assertByteArrayEqual(actData, expData, "lob data is wrong");
            
         // read mode
         byte[] seekReadData = new byte[lobSize];
         try (DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_READ)) {
             lob.seek(offset, DBLob.SDB_LOB_SEEK_SET);
             lob.read(seekReadData);
         }
            
         byte[] seekExpData = Arrays.copyOfRange(expData, offset, offset + lobSize);
         RandomWriteLobUtil.assertByteArrayEqual(seekReadData, seekExpData, "lob data is wrong");        
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
