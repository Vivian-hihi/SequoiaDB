package com.sequoiadb.lob.randomwrite;

import java.util.Arrays;

import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.lob.randomwrite.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @FileName seqDB-13390 : 指定不同lob大小truncate 
 * @Author linsuqiang
 * @Date 2017-11-16
 * @Version 1.00
 */

/*
 * 1、指定oid执行truncateLob操作，删除超过指定长度部分的数据，其中length分别验证如下场景： 
 *    a、length等于lob大小 
 *    b、length大于lob大小 
 *    c、length小于lob大小（length为0，truncate所有数据；
 *       length为1，truncate超过1byte数据；
 *       truncate数据大小为1btye数据） 
 * 2、检查操作结果（读取lob，查看lob对象长度，执行listLobs查看lobsize信息）
 */

public class LengthTest13390 extends SdbTestBase {

    private final String csName = "lobTruncate13390";
    private final String clName = "lobTruncate13390";
    private final int lobPageSize = 64 * 1024; // 128k
    
    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    
    @BeforeClass
    public void setUp() {

        try {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            
            // create cs cl
            BSONObject csOpt = (BSONObject) JSON.parse("{LobPageSize: " + lobPageSize + "}");
            cs = sdb.createCollectionSpace(csName, csOpt);
            BSONObject clOpt = (BSONObject) JSON.parse("{ShardingKey:{a:1},ShardingType:'hash'}");
            cl = cs.createCollection(clName, clOpt);
            
        } catch (BaseException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        }
    }
    
    // a、length等于lob大小 
    @Test
    public void lenEqSize() {
        try {
            int lobSize = lobPageSize;
            byte[] data = RandomWriteLobUtil.getRandomBytes(lobSize);
            ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, data);
            byte[] expData = data;
            
            cl.truncateLob(oid, lobSize);
            
            byte[] actData = RandomWriteLobUtil.readLob(cl, oid);
            RandomWriteLobUtil.assertByteArrayEqual(actData, expData, "lob data is wrong");
            long actSize = getSizeByListLobs(cl, oid);
            Assert.assertEquals(actSize, lobSize, "wrong length after truncate lob");
            
        } catch (BaseException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        }
    }
    
    // b、length大于lob大小 
    @Test
    public void lenGtSize() {
        try {
            int lobSize = lobPageSize;
            byte[] data = RandomWriteLobUtil.getRandomBytes(lobSize);
            ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, data);
            byte[] expData = data;
            
            int length = 2 * lobPageSize;
            cl.truncateLob(oid, length);
            
            byte[] actData = RandomWriteLobUtil.readLob(cl, oid);
            RandomWriteLobUtil.assertByteArrayEqual(actData, expData, "lob data is wrong");
            long actSize = getSizeByListLobs(cl, oid);
            Assert.assertEquals(actSize, lobSize, "wrong length after truncate lob");
            
        } catch (BaseException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        }
    }
    
    // c、length小于lob大小（length为0，truncate所有数据；
    @Test
    public void lenZero() {
        try {
            int lobSize = lobPageSize;
            byte[] data = RandomWriteLobUtil.getRandomBytes(lobSize);
            ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, data);
            
            cl.truncateLob(oid, 0);
            byte[] expData = new byte[0];
            
            byte[] actData = RandomWriteLobUtil.readLob(cl, oid);
            RandomWriteLobUtil.assertByteArrayEqual(actData, expData, "lob data is wrong");
            long actSize = getSizeByListLobs(cl, oid);
            Assert.assertEquals(actSize, 0, "wrong length after truncate lob");
            
        } catch (BaseException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        }
    }
    
    // length为1，truncate超过1byte数据；
    @Test
    public void lenLtSize1() {
        try {
            int lobSize = lobPageSize;
            byte[] data = RandomWriteLobUtil.getRandomBytes(lobSize);
            ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, data);
            byte[] expData = data;
            
            cl.truncateLob(oid, 1); 
            expData = Arrays.copyOfRange(expData, 0, 1);
            
            byte[] actData = RandomWriteLobUtil.readLob(cl, oid);
            Assert.assertEquals(actData[0], expData[0], "lob content is wrong");
            long actSize = getSizeByListLobs(cl, oid);
            Assert.assertEquals(actSize, 1, "wrong length after truncate lob");
            
        } catch (BaseException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        }
    }
    
    // truncate数据大小为1btye数据） 
    @Test
    public void lenLtSize2() {
        try {
            int lobSize = lobPageSize;
            byte[] data = RandomWriteLobUtil.getRandomBytes(lobSize);
            ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, data);
            byte[] expData = data;
            
            int length = lobSize - 1;
            cl.truncateLob(oid, length);
            expData = Arrays.copyOfRange(expData, 0, length);
            
            byte[] actData = RandomWriteLobUtil.readLob(cl, oid);
            RandomWriteLobUtil.assertByteArrayEqual(actData, expData, "lob data is wrong");
            long actSize = getSizeByListLobs(cl, oid);
            Assert.assertEquals(actSize, length, "wrong length after truncate lob");
            
        } catch (BaseException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        }
    }
    
    @AfterClass
    public void tearDown() {
        try {
            if (sdb.isCollectionSpaceExist(csName)) {
                sdb.dropCollectionSpace(csName);
            }
        } catch (BaseException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        } finally {
            if (null != sdb) {
                sdb.close();
            }
        }
    }

    private long getSizeByListLobs(DBCollection cl, ObjectId oid) {
        DBCursor cursor = null;
        long lobSize = 0;
        boolean oidFound = false;
        try {
            cursor = cl.listLobs();
            while(cursor.hasNext()) {
                BSONObject res = cursor.getNext();
                ObjectId curOid = (ObjectId) res.get("Oid");
                if (curOid.equals(oid)) {
                    lobSize = (long) res.get("Size");
                    oidFound = true;
                    break;
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        if (!oidFound) {
            throw new RuntimeException("no such oid");
        }
        return lobSize;
    }
}
