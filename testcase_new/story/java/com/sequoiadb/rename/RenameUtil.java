package com.sequoiadb.rename;

import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.bson.types.ObjectId;
import org.testng.Assert;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Description RenameUtil.java
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameUtil extends SdbTestBase {
	
	public static void checkRenameCSResult(Sequoiadb db, String oldCSName, String newCSName, int clNum){
		
		try{
			db.getCollectionSpace(oldCSName);
			Assert.fail("cs it's been renamed, It shouldn't exist");
		}catch(BaseException e){
			if(e.getErrorCode() != -34){
				throw e;
			}
		}
		
		DBCursor cur = null;
		try{
			cur = db.getSnapshot(Sequoiadb.SDB_SNAP_COLLECTIONSPACES, "{'Name':'"+newCSName+"'}", "", "");
			BSONObject obj = cur.getNext();
			BasicBSONList cls = (BasicBSONList) obj.get("Collection");
			if(cls.size()!=clNum){
				Assert.fail("cl count error, exp: "+ clNum +",act :" + cls.size());
			}
			for (int i = 0; i < cls.size(); i++) {
				BSONObject ele = (BSONObject) cls.get(i);
				String name = (String) ele.get("Name");
				String csName = name.split("\\.")[0];
				if(!csName.equals(newCSName)){
					Assert.fail("cs name contrast error");
				}
			}
		}finally{
			if(cur != null){
				cur.close();
			}
		}
		
	}
	
	public static List<ObjectId> putLob(DBCollection cl, byte[] data, int lobNum){
		
		List<ObjectId> idList = new ArrayList<ObjectId>();
		for (int i = 0; i < lobNum; i++) {
			DBLob lob = null;
			try {
				lob = cl.createLob();
				lob.write(data);
				idList.add(lob.getID());
			} finally {
				lob.close();
			}
		}
		return idList;
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
	
	public static void insertData(DBCollection cl, int recordNum){
		
		List<BSONObject> data = new ArrayList<BSONObject>();
		for (int i = 0; i < recordNum/1000; i++) {
			for (int j = 0; j < 1000; j++) {
				BSONObject record = new BasicBSONObject();
				record.put("a", i*1000+j);
				record.put("no", "No."+i*1000+j);
				record.put("phone", 13700000000L + i*1000+j);
				record.put("text", "Test ReName, This is the test statement used to populate the data");
				data.add(record);
			}
			cl.insert(data);
		}
	}
}
