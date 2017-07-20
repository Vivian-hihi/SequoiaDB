package com.sequoiadb.cappedCL;

import java.util.ArrayList; 
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;

public class Commlib {
	
	/**
	 * @param sdb
	 * @param csName
	 * @param clName
	 * @throws BaseException
	 */
	public static DBCollection createCL(Sequoiadb sdb,String csName,String clName,boolean isCapped) throws BaseException{
		try {
			BSONObject options_cs = new BasicBSONObject();
			options_cs.put("Capped", true);
//			System.out.println("options_cs: " + options_cs);
//			System.err.println(sdb);
			if(isCapped == true) {
				sdb.createCollectionSpace(csName,options_cs);//cappedCL
			}else {
				sdb.createCollectionSpace(csName);//commonCL
			}
			
		}catch (BaseException e) {
			if(-33 != e.getErrorCode()) {
				throw e;
			}
		}
		DBCollection cl = null;
		BSONObject options_cl = new BasicBSONObject();
		options_cl.put("Capped", true);
		options_cl.put("Size", 8589934592L);
		options_cl.put("AutoIndexId", false);
		try {
			CollectionSpace cs = sdb.getCollectionSpace(csName);
			if(cs.isCollectionExist(clName)) {
//				System.out.println("options_cl has existed");
				cs.dropCollection(clName);
			}
			if(isCapped == true) {
				cl = cs.createCollection(clName,options_cl);
			}else {
				cl = cs.createCollection(clName,(BSONObject) JSON.parse("{AutoIndexId:false}"));
			}
			
		}catch (BaseException e) {
			if(-22 != e.getErrorCode()) {
				throw e;
			}
			throw e;
		}
		return cl;
	}
	
	/**
	 * @param sdb
	 * @param csName
	 * @param clName
	 * @throws BaseException
	 */
	public static List<DBCollection> createMoreCappedCL(Sequoiadb sdb,String csName,String clName,int csNum,int clNum) throws BaseException{
		List<DBCollection> dbCollections = new ArrayList<DBCollection>();
		
		try {
			for(int csNo = 1; csNo <= csNum; csNo++) {
				BSONObject options_cs = new BasicBSONObject();
				options_cs.put("Capped", true);
				sdb.createCollectionSpace(csName + csNo,options_cs);
				
				CollectionSpace cs = sdb.getCollectionSpace(csName + csNo);
				for(int clNo = 1 ; clNo <= clNum; clNo++) {
					if(cs.isCollectionExist(clName + clNo)) {
						cs.dropCollection(clName + clNo);
					}
					BSONObject options_cl = new BasicBSONObject();
					options_cl.put("Capped", true);
					options_cl.put("Size", 8589934592L);
					options_cl.put("AutoIndexId", false);
					DBCollection cl = cs.createCollection(clName + clNo,options_cl);
					dbCollections.add(cl);
				}
			}
		}catch (BaseException e) {
			if(-33 != e.getErrorCode() || -22 != e.getErrorCode()) {
				throw e;
			}
		}
	
		return dbCollections;
	}
	
	/**
	 * @param cl
	 * @param strBuffer
	 * @param obj
	 * @throws BaseException
	 */
	public static void insertRecords(DBCollection cl,StringBuffer strBuffer,BSONObject obj) throws BaseException{
		
        final int each_thread_recordNums = 20;
		try {	
	       for(int i = 0; i < each_thread_recordNums; i++) {
	          cl.insert(obj);
//	          System.out.println("Thread: " + Thread.currentThread().getName() + " inserting record length " + strBuffer.length());
	       }
	         
		} catch (BaseException e) {
			throw e;	
		}
	}
	
	/**
	 * get random record length
	 */
	public static int getRandomStringLength() {
		int minLength = 1 * 1024 * 1024;
		int range = 1 * 1024  ;//strings range [1m,1m+1k]
        int stringLength =  (int)(minLength + Math.random() * range);
        return stringLength;
	}
	
	/**
	 * check whether LogicalID is correctly
	 * @param sdb
	 * @param cl
	 * @param stringLength
	 * @throws BaseException
	 */
	public static boolean checkLogicalID(Sequoiadb sdb,DBCollection cl,int stringLength) throws BaseException{
		System.out.println("--------begin to check logicalId---------");
		DBCursor queryCursor = null;
		try {
			sdb.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));
			final int each_add_55 = 55;
			final int full_byte_4 = 4; //4 bytes
		    int blockCounts = 1; //init block 
			final int block_max_32 = 33554396; //each block is up to 32m
			long expectId = 0;
			queryCursor = cl.query((BSONObject)null, null, null, null);
			while(queryCursor.hasNext()) {
				long actId = (long)queryCursor.getNext().get("_id");
				int recordLength = stringLength + each_add_55;
				
				recordLength = (0 == recordLength % full_byte_4)?
						recordLength : (recordLength - recordLength % full_byte_4 + full_byte_4);
		       
				long nextRecordId = expectId + recordLength;
				if(nextRecordId > (blockCounts * block_max_32)) { //if the next record length is up to current block size,it will be put to the next block
					expectId = blockCounts * block_max_32;
					++blockCounts;
				}
				
		        if(expectId != actId) {
		        	return false;
		        }
		        expectId = actId + recordLength;//This expectId belongs to the next record			
			}
			return true;
		}catch(BaseException e){
//			System.out.println("check Exception: " + e.getMessage());
			return false;
		}finally {
//			System.out.println("--------end to check logicalId---------");
			queryCursor.close();
		}
		
	}
}
