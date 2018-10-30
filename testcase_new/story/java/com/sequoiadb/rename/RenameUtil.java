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

import com.sequoiadb.base.CollectionSpace;
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
				throw e;//TODO:建议出错给些提示信息吧，比较容易看出时哪个操作出错
			}
		}
		//TODO:这段代码逻辑写的太复杂了，阅读起来也比较费力，建议拆分下，有些方法可以提取出来，另外要加上注释信息
		if (clNum != 0) {
			DBCursor cur = null;
			int times = 0;
			for (int k = 0; k < 50; k++) {
				try {
					cur = db.getSnapshot(Sequoiadb.SDB_SNAP_COLLECTIONSPACES, "{'Name':'" + newCSName + "'}", "", "");
					if (!cur.hasNext()) {
						Assert.fail("cs it's not exist, csName: " + newCSName);
					}
	
					BSONObject obj = cur.getNext();
					BasicBSONList cls = (BasicBSONList) obj.get("Collection");
					if (cls.size() != clNum) {
						times++;
						if(times == 50){
							Assert.fail("cl count error, exp: " + clNum + ",act :" + cls.size());
						}
						try {
							Thread.sleep(100);
						} catch (InterruptedException e) {
							Assert.fail(e.getMessage());
						}
						continue;
					}
					for (int i = 0; i < cls.size(); i++) {
						BSONObject ele = (BSONObject) cls.get(i);
						String name = (String) ele.get("Name");
						String csName = name.split("\\.")[0];
						if (!csName.equals(newCSName)) {
							Assert.fail("cs name contrast error");
						}
					}
				} finally {
					if (cur != null) {
						cur.close();
					}
				}
				break;
			}
		}else{
			try {
				db.getCollectionSpace(newCSName);
			} catch (BaseException e) {
				Assert.fail("afresh get cs failure, error:"+e);
			}
		}
	}
	
	public static void checkRenameCLResult(Sequoiadb db, String csName, String oldCLName, String newCLName){
		
		CollectionSpace cs = db.getCollectionSpace(csName);
		try {
			cs.getCollection(oldCLName);
		} catch (BaseException e) {
			if(e.getErrorCode() != -23){
				throw e;//TODO:建议出错给些提示信息吧，比较容易看出时哪个操作出错
			}
		}
		//TODO:游标名建议给个有含义的变量名，不要直接写cur
		DBCursor cur = null;
		try {//TODO:多处用到csName+newCLName，建议定义一个fullName名
			cur = db.getSnapshot(Sequoiadb.SDB_SNAP_COLLECTIONS, "{'Name':'" + csName + "." + newCLName + "'}", "", "");
			if(!cur.hasNext()){
				Assert.fail("cl is not exist, clFullName: " + csName + "." + newCLName );
			}
			while(cur.hasNext()){
				BSONObject obj = cur.getNext();
				String name = (String) obj.get("Name");
				if(!name.equals(csName + "." + newCLName)){
					Assert.fail("cl fullname error, exp: " + csName + "." + newCLName +", act: "+name);
				}
			}
		} finally{
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
				lob.write(data);//TODO:lobId可以直接取返回值，不需要再get一次
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
		//TODO：这个小于1的判断没有必要吧，建议给个取默认值的接口
		if(recordNum < 1){
			recordNum = 1;
		}
		//TODO:插入数据的代码建议简化下，这里分了两步操作，应该加下描述信息
		int times = recordNum/1000;
		for (int i = 0; i < times; i++) {
			List<BSONObject> data = new ArrayList<BSONObject>();
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
		
		List<BSONObject> dataA = new ArrayList<BSONObject>();
		for(int k = 0; k < recordNum%1000; k++){
			BSONObject record = new BasicBSONObject();
			record.put("a", times*1000+k);
			record.put("no", "No."+times*1000+k);
			record.put("phone", 13700000000L + times*1000+k);
			record.put("text", "Test ReName, This is the test statement used to populate the data");
			dataA.add(record);
		}
		if(dataA.size()!=0){
			cl.insert(dataA);
		}
	}
	
//	public static void checkRecord(DBCollection cl, int begineNum, int endNum){
//		List<BSONObject> expList = new ArrayList<BSONObject>();
//		List<BSONObject> actList = new ArrayList<BSONObject>();
//		for (int i = begineNum; i < endNum; i++) {
//			BSONObject record = new BasicBSONObject();
//			record.put("a", i);
//			record.put("no", "No." + i);
//			record.put("phone", 13700000000L + i);
//			record.put("text", "Test ReName, This is the test statement used to populate the data");
//			expList.add(record);
//		}
//		DBCursor cur = cl.query(null, null, new BasicBSONObject("a", 1), null);
//		while(cur.hasNext()){
//			actList.add(cur.getNext());
//		}
//		cur.close();
//		Assert.assertEquals(actList, expList);
//	}
	
	//TODO:方法名描述有误，应该是Exist，判断cl是否存在，直接有接口可以使用，不需要再另外实现吧
	public static void checkCLExit(Sequoiadb db, String csName, String clName, boolean clIsExist){
		DBCursor cur = null;
		try {
			cur = db.getSnapshot(Sequoiadb.SDB_SNAP_COLLECTIONS, "{'Name':'" + csName + "." + clName + "'}", "", "");
			
			if(clIsExist){
				if(!cur.hasNext()){
					Assert.fail("cl is not exist, clFullName: " + csName + "." + clName );
				}
				while(cur.hasNext()){
					BSONObject obj = cur.getNext();
					String name = (String) obj.get("Name");
					if(!name.equals(csName + "." + clName)){
						Assert.fail("cl fullname error, exp: " + csName + "." + clName +", act: "+name);
					}
				}
			}else{
				if(cur.hasNext()){
					Assert.fail("cl is exist, clFullName: " + csName + "." + clName );
				}
				try {
					db.getCollectionSpace(csName).getCollection(clName);
				} catch (BaseException e) {
					if(e.getErrorCode() != -23){
						throw e;
					}
				}
			}
		} finally{
			if(cur != null){
				cur.close();
			}
		}
	} 
	
	//TODO:这种检查切分的方法不严谨，如果要查看编目信息检查结果，需要判断组名还有切分范围
	public static void checkSplitResult(Sequoiadb db, String csName, String clName, List<String> groups){
		
		DBCursor cur = db.getSnapshot(Sequoiadb.SDB_SNAP_CATALOG, new BasicBSONObject("Name", csName+"."+clName), null, null);
		if(!cur.hasNext()){
			Assert.fail("cl is not exist, " + csName + "." + clName);
		}
		BSONObject obj = cur.getNext();
		BasicBSONList cataInfo = (BasicBSONList) obj.get("CataInfo");
		if(cataInfo.size() != groups.size()){
			Assert.fail("cataInfo error: exp: " +groups.toString() +" act: " + cataInfo.toString());
		}
		for (int i = 0; i < cataInfo.size(); i++) {
			BSONObject info = (BSONObject) cataInfo.get(i);
			String groupName = (String) info.get("GroupName");
			if(!groups.contains(groupName)){
				Assert.fail("groupName error: exp: " +groups.toString() +" act: " + cataInfo.toString());
			}
		}
	}
	
}
