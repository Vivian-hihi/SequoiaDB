package com.sequoiadb.lob.randomwrite;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.SdbTestBase;
import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;
import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.logging.Logger;

import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.*;

/**
 * FileName: LobTest13228.java
 * test content:
 * 测试用例 seqDB-13228 :
 * 1、打开lob（创建模式新建lob）
 * 2、通过seek指定偏移量，偏移量分别验证如下场景：
 * a、同一个数据页内偏移写
 * b、跨多个数据页偏移写
 * c、偏移位置覆盖起始、中间、末尾（SDB_LOB_SEEK_SET/SDB_LOB_SEEK_CUR/SDB_LOB_SEEK_END）
 * 3、写入lob数据
 * 4、检查lob写入结果
 * 1、写入lob成功，查询lob数据信息和实际写入数据一致（比较MD5值）
 * 2、执行list查看显示lob size信息正确，包含seek size长度
 * * testlink case:seqDB-13228
 * @author laojingtang
 * @UpdateAuthor wangkexin
 * @Date    2017.11.22
 * @UpdateDate 2018.08.15
 * @version 1.10
 */
public class LobTest13228 extends SdbTestBase {
    Logger log = Logger.getLogger(LobTest13237.class.getName());
    Sequoiadb db = null;
    DBCollection dbcl = null;
    CollectionSpace cs = null;
    String csName;
    String clName;

    @BeforeClass
    public void setupClass() {
        csName = SdbTestBase.csName;
        clName = "cl_" + this.getClass().getSimpleName();
        db = new Sequoiadb(coordUrl, "", "");
        cs = db.getCollectionSpace(csName);
        dbcl = cs.createCollection(clName,
                		(BSONObject) JSON.parse("{ShardingKey:{\"_id\":1},ShardingType:\"hash\"}"));
    }

    @AfterClass
    public void afterClass() {
        cs.dropCollection(clName);
        db.close();
    }

    @DataProvider(name = "testLob13228DataProvider")
    public static Object[][] testLob13228DataProvider() {
        return new Object[][]{
    		//同一个数据页内偏移写
            {200 * 1024 , 10 * 1024, DBLob.SDB_LOB_SEEK_SET},
            {100 * 1024, 20 * 1024, DBLob.SDB_LOB_SEEK_CUR},
            {100 * 1024, 50 * 1024, DBLob.SDB_LOB_SEEK_END},
            
            //跨多个数据页偏移写
            {1024 * 1024, 100 * 1024, DBLob.SDB_LOB_SEEK_SET},
            {1024 * 1024, 50 * 1024, DBLob.SDB_LOB_SEEK_CUR},
            {1024 * 1024, 40 * 1024, DBLob.SDB_LOB_SEEK_END},
        };
    }

    @Test(dataProvider = "testLob13228DataProvider")
    public void testLob13228(int appendDataSize, int seekSize, int seekType) {
    	//第一种情况：创建lob后先向lob中写数据，再进行偏移写操作
    	testLob13228a(appendDataSize ,seekSize ,seekType); 
    	
    	//第二种情况：创建lob后直接进行偏移写操作
    	testLob13228b(appendDataSize ,seekSize ,seekType); 
    }
    
    public void testLob13228a(int appendDataSize, int seekSize, int seekType) {
    	//设置初始数据大小100kb
    	int initDataSize = 100 * 1024;	
    	
    	ObjectId id = ObjectId.get();
    	DBLob lob = dbcl.createLob(id);
    	
    	byte[] initData = getRandomBytes(initDataSize);
        String initMd5 = getMd5(initData);
    	
        byte[] appendData = getRandomBytes(appendDataSize);
        String appendMd5 = getMd5(appendData);
        
		lob.write(initData);
        
    	lob.seek(seekSize, seekType);	
        lob.write(appendData);
        lob.close();
        
        //偏移写初始位置
        int appendPosition;
        byte[] actual = readLob(dbcl, id);
        
        if (seekType == DBLob.SDB_LOB_SEEK_CUR) {
            appendPosition = seekSize + initDataSize;
        }
        else if (seekType == DBLob.SDB_LOB_SEEK_END){
        	System.out.println(lob.getSize());
            appendPosition = initDataSize - seekSize;
            System.out.println(appendData.length);
        }
        else{
            appendPosition = seekSize;
        }

        if (appendPosition > initDataSize) { 
        	String previnitMd5 = getMd5(Arrays.copyOfRange(actual, 0, initDataSize));
            Assert.assertEquals(previnitMd5, initMd5);
            
            String prevappendMd5 = getMd5(Arrays.copyOfRange(actual, appendPosition, appendPosition + appendDataSize));
            Assert.assertEquals(prevappendMd5, appendMd5);
            
            //比较lob size 信息是否正确
            Assert.assertEquals(lob.getSize(),initDataSize + seekSize + appendDataSize);
        } else{	
        	String prevappendMd5 = getMd5(Arrays.copyOfRange(actual, appendPosition, appendPosition + appendDataSize));
        	Assert.assertEquals(prevappendMd5,appendMd5);
        	
        	//比较lob size 信息是否正确
        	if(seekType == DBLob.SDB_LOB_SEEK_SET){
        		Assert.assertEquals(lob.getSize(),appendDataSize + seekSize);
        	}else{
        		Assert.assertEquals(lob.getSize(),initDataSize - seekSize + appendDataSize);
        	}
        }
    }
    public void testLob13228b(int appendDataSize, int seekSize, int seekType) {
    	ObjectId id = ObjectId.get();
    	DBLob lob = dbcl.createLob(id);
    	
        byte[] appendData = getRandomBytes(appendDataSize);
        String appendMd5 = getMd5(appendData);
        
        if(seekType == DBLob.SDB_LOB_SEEK_END){
        	lob.seek(0, seekType);
        }else{
        	lob.seek(seekSize, seekType);	
        }
        lob.write(appendData);
        lob.close();
      
        byte[] actual = readLob(dbcl, id);
        
        //check the result
        if(seekType != DBLob.SDB_LOB_SEEK_END){
        	String previnitMd5 = getMd5(Arrays.copyOfRange(actual, seekSize, seekSize + appendDataSize));
        	Assert.assertEquals(appendMd5, previnitMd5);
        	
            Assert.assertEquals(lob.getSize(),seekSize + appendDataSize);
        } else{
        	String previnitMd5 = getMd5(actual);
        	Assert.assertEquals(appendMd5, previnitMd5);
        	
        	Assert.assertEquals(lob.getSize(),appendDataSize);
        }
    }
    public static String getMd5(Object inbuff){
        MessageDigest md5 = null;
        String value = "";
        
        try {
            md5 = MessageDigest.getInstance("MD5");
            if(inbuff instanceof ByteBuffer){
                md5.update((ByteBuffer)inbuff);
            }else if(inbuff instanceof String){
                md5.update(((String)inbuff).getBytes());
            }else if(inbuff instanceof byte[]){
            	md5.update((byte[]) inbuff);
            }else{
            	Assert.fail("invalid parameter!");
            }
            BigInteger bi = new BigInteger(1, md5.digest());
            value = bi.toString(16);
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
            Assert.fail("fail to get md5!"+e.getMessage());
        }
        return value;
    }
}
