package com.sequoiadb.lob.randomwrite;


import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @FileName seqDB-13250: 写入空切片数据 
 * @Author linsuqiang
 * @Date 2017-11-08
 * @Version 1.00
 */

/*
 * 1、创建cs，设置lobpagesize大小，创建cl 
 * 2、写入空lob 
 * 3、打开已存在lob，seek、lock指定偏移量锁定数据段，锁定数据段存在空切片范围，分别验证如下场景： 
 *     a、元数据片（0片）不带数据，偏移位置从第1片开始 
 *     b、元数据和数据之间有空块（如偏移位置从0片中间位置开始锁定）
 *     c、数据切片未写满（结尾有空段、起始位置为有空段） 
 *     d、数据页结尾1kb空块 
 *     e、数据页起始1kb空块 
 *     f、空块为一完整数据页 
 * 3、写入lob 
 * 4、读取lob信息，检查操作结果
 */

public class TestEmptyPiece13250 extends SdbTestBase {

    private final String csName = "writelob13250";
    private final String clName = "writelob13250";
    private final int lobMetaSize = 1024; // 1k
    private final int lobPageSize = 32 * 1024; // 32k
    
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

    @DataProvider
    public Object[][] rangeProvider() {
        Object[][] result = null;
        int firstDataPagePos = lobPageSize - lobMetaSize;
        result = new Object[][]{
            // a、元数据片（0片）不带数据，偏移位置从第1片开始 
            new Object[]{new LobPart(firstDataPagePos, lobPageSize)},
            // b、元数据和数据之间有空块（如偏移位置从0片中间位置开始锁定）
            new Object[]{new LobPart(lobPageSize / 2, lobPageSize)},
            // c、数据切片未写满（结尾有空段、起始位置为有空段） 
            new Object[]{new LobPart(firstDataPagePos + 1024, lobPageSize - 1024)},
            // d、数据页结尾1b空块 
            new Object[]{new LobPart(firstDataPagePos, lobPageSize - 1)},
            // e、数据页起始1b空块 
            new Object[]{new LobPart(firstDataPagePos + 1, lobPageSize)},
            // f、空块为一完整数据页 
            new Object[]{new LobPart(firstDataPagePos + lobPageSize, lobPageSize)}
        };
        return result;
    }
    
    @Test(dataProvider="rangeProvider")
    public void testLob(LobPart part) {
    	byte[] data = new byte[0];
        ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, data);
        byte[] expData = data;
            
        try (DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)) {
            lockAndSeekAndWriteLob(lob, part);
            expData = updateExpData(expData, part);
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
        }finally {
            if (null != sdb) {
                sdb.close();
            }
        }
    }
    
    private void lockAndSeekAndWriteLob(DBLob lob, LobPart part) {
        lob.lock(part.getOffset(), part.getLength());
        lob.seek(part.getOffset(), DBLob.SDB_LOB_SEEK_SET);
        lob.write(part.getData());
    }
    
    private byte[] updateExpData(byte[] expData, LobPart part) {
        return RandomWriteLobUtil.appendBuff(expData, part.getData(), part.getOffset());
    }

}
