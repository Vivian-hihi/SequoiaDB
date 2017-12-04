package com.sequoiadb.lob.randomwrite;


import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @FileName seqDB-13317: 加锁写lob过程中执行切分 
 * @Author linsuqiang
 * @Date 2017-11-08
 * @Version 1.00
 */

/*
 * 1、共享模式下，并发执行如下操作
 *    (1)打开已存在lob对象，seek指定偏移范围，执行lock锁定数据段，向锁定数据段写入lob， 
 *    重复多次加锁不同范围写lob
 *    （2）执行切分操作（异步切分），其中切分过程中需要覆盖元数据片迁移到目标组场景
 * 2、读取lob，检查操作结果
 */

public class WriteAndSplit13317 extends SdbTestBase {

    private final String csName = "writelob13317";
    private final String clName = "writelob13317";
    private final int lobPageSize = 16 * 1024; // 16k
    
    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    
    @BeforeClass
    public void setUp() {

        try {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            CommLib comm = new CommLib(); // from package testcommon
            if (comm.isStandAlone(sdb) || comm.OneGroupMode(sdb)) {
                throw new SkipException("no groups to split");
            }
            
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

    @Test
    public void testLob() {
        try {
            int lobSize = 2 * 1024 * 1024;
            byte[] data = RandomWriteLobUtil.getRandomBytes(lobSize);
            ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, data);
            byte[] expData = data;
            
            LobPart partA = new LobPart(0          , 256 * 1024);
            LobPart partB = new LobPart(256  * 1024, 768 * 1024);
            LobPart partC = new LobPart(1024 * 1024, 512 * 1024);
            
            String srcGroupName = RandomWriteLobUtil.getSrcGroupName(sdb, csName, clName);
            String dstGroupName = RandomWriteLobUtil.getSplitGroupName(sdb, srcGroupName);
            long taskId = 0;
            
            DBLob lob = null;
            try {
                taskId = cl.splitAsync(srcGroupName, dstGroupName, 50);
                lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE);
                
                lockAndSeekAndWriteLob(lob, partA);
                lockAndSeekAndWriteLob(lob, partB);
                lockAndSeekAndWriteLob(lob, partC);
                
                expData = updateExpData(expData, partA);
                expData = updateExpData(expData, partB);
                expData = updateExpData(expData, partC);
            } finally {
                if (null != lob) {
                    lob.close();
                }
            }
            
            long[] taskIdArr = {taskId};
            sdb.waitTasks(taskIdArr);
            
            byte[] actData = RandomWriteLobUtil.readLob(cl, oid);
            RandomWriteLobUtil.assertByteArrayEqual(actData, expData, "lob data is wrong");
            
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
    
    private void lockAndSeekAndWriteLob(DBLob lob, LobPart part) {
        lob.lockAndSeek(part.getOffset(), part.getLength());
        lob.write(part.getData());
    }
    
    private byte[] updateExpData(byte[] expData, LobPart part) {
        return RandomWriteLobUtil.appendBuff(expData, part.getData(), part.getOffset());
    }

}
