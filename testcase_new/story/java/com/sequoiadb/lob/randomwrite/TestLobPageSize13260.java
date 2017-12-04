package com.sequoiadb.lob.randomwrite;


import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @FileName seqDB-13242: 锁定多个数据范围后写入lob 
 * @Author linsuqiang
 * @Date 2017-11-08
 * @Version 1.00
 */

/*
 * 1、创建CS，其中lob数据页大小分别取值为0-524288之间的所有值 
 * 2、创建cl，写入lob 
 * 3、打开已存在lob 
 * 4、seekAndlock指定偏移长度锁定数据段，分别验证如下场景： 
 *     a、锁定数据范围不连续，存在空切片 
 *     b、从结束位置锁定数据段，顺序写lob 
 *     c、锁定数据范围连续，指定长度覆盖写lob 
 * 5、读取lob，检查操作结果
 */

public class TestLobPageSize13260 extends SdbTestBase {

    private final String csName = "writelob13260";
    private final String clName = "writelob13260";
    
    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    
    @BeforeClass
    public void setUp() {

        try {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        }
    }
    
    @DataProvider
    public Object[][] pageSizeProvider() {
        Object[][] result = null;
        // all legal LobPageSize
        result = new Object[][]{
            new Object[]{0},
            new Object[]{4 * 1024},
            new Object[]{8 * 1024},
            new Object[]{16 * 1024},
            new Object[]{32 * 1024},
            new Object[]{64 * 1024},
            new Object[]{128 * 1024},
            new Object[]{256 * 1024},
            new Object[]{512 * 1024},
        };
        return result;
    }
    
    @Test(dataProvider="pageSizeProvider")
    public void testLob(int lobPageSize) {
        try {
            // create cs cl
            BSONObject csOpt = (BSONObject) JSON.parse("{LobPageSize: " + lobPageSize + "}");
            cs = sdb.createCollectionSpace(csName, csOpt);
            BSONObject clOpt = (BSONObject) JSON.parse("{ShardingKey:{a:1},ShardingType:'hash'}");
            cl = cs.createCollection(clName, clOpt);
            
            int lobSize = 300 * 1024;
            byte[] data = RandomWriteLobUtil.getRandomBytes(lobSize);
            ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, data);
            byte[] expData = data;
            
            // a、锁定数据范围不连续，存在空切片
            lobPageSize = (lobPageSize == 0) ? (256 * 1024) : lobPageSize; // zero means default
            try (DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)) {
                LobPart partA = new LobPart(0, lobPageSize);
                LobPart partB = new LobPart(4 * lobPageSize, 2 * lobPageSize);
                LobPart partC = new LobPart(7 * lobPageSize, lobPageSize);
                
                lockAndSeekAndWriteLob(lob, partA);
                lockAndSeekAndWriteLob(lob, partB);
                lockAndSeekAndWriteLob(lob, partC);
                
                expData = updateExpData(expData, partA);
                expData = updateExpData(expData, partB);
                expData = updateExpData(expData, partC);
            }
            byte[] actData = RandomWriteLobUtil.readLob(cl, oid);
            RandomWriteLobUtil.assertByteArrayEqual(actData, expData, "lob data is wrong");
            
            // b、从结束位置锁定数据段，顺序写lob 
            try (DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)) {
                LobPart part = new LobPart((int) lob.getSize(), 2 * lobPageSize);
                lockAndSeekAndWriteLob(lob, part);
                expData = updateExpData(expData, part);
            }
            actData = RandomWriteLobUtil.readLob(cl, oid);
            RandomWriteLobUtil.assertByteArrayEqual(actData, expData, "lob data is wrong");
            
            // c、锁定数据范围连续，指定长度覆盖写lob 
            try (DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)) {
                LobPart part = new LobPart((int) (lob.getSize() / 2), 2 * lobPageSize);
                lockAndSeekAndWriteLob(lob, part);
                expData = updateExpData(expData, part);
            }
            actData = RandomWriteLobUtil.readLob(cl, oid);
            RandomWriteLobUtil.assertByteArrayEqual(actData, expData, "lob data is wrong");
            
        } catch (BaseException e) {
            e.printStackTrace();
            Assert.fail("lobPageSize:" + lobPageSize + "\n" + e.getMessage());
        } finally {
            if (sdb.isCollectionSpaceExist(csName)) {
                sdb.dropCollectionSpace(csName);
            }
        }
    }

    @AfterClass
    public void tearDown() {
        if (null != sdb) {
            sdb.close();
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
