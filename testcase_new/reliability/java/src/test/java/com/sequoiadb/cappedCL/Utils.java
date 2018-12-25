package com.sequoiadb.cappedCL;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;

import java.util.Random;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.GroupMgr ;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import org.bson.types.ObjectId;
import org.testng.Assert;

public class Utils {

	/**
	* check whether all data nodes is consistent
	*/
	public static void checkConsistency(GroupMgr groupMgr, GroupWrapper dataGroup) throws ReliabilityException{
	    int groupId = dataGroup.getGroupID() ;
	    groupMgr.refresh() ;
	    dataGroup  = groupMgr.getGroupById( groupId ) ;
        List<String> urls = dataGroup.getAllUrls();
        List<List<BSONObject>> resultList = new ArrayList<List<BSONObject>>();
        // get datagroup info from all data nodes
        for (String url : urls) {
            Sequoiadb dataDB = new Sequoiadb(url, "", "");
            DBCursor cursor = dataDB.listCollections();
            List<BSONObject> eachNodeCLNameList = new ArrayList<BSONObject>();
            while (cursor.hasNext()) {
            	eachNodeCLNameList.add(cursor.getNext());
            }
            cursor.close();
            resultList.add(eachNodeCLNameList);
            dataDB.close();
        }
        // check data count
        if (resultList.get(0).size() != resultList.get(1).size() || resultList.get(1).size() != resultList.get(2).size()) {
            System.out.println("data count is different between data nodes! ");
            System.out.println(resultList.get(0).size() + " " + resultList.get(1).size() + " " + resultList.get(2).size());
            throw new ReliabilityException("checkConsistency failed. ");
        }
        // check data nodes content
        List<BSONObject> srcList = resultList.get(0);
        for (int i = 1; i < resultList.size(); i++) {
            List<BSONObject> destList = resultList.get(i);
            for (int j = 0; j < srcList.size(); j++) {
                if (!srcList.get(j).equals(destList.get(j))) {
                    System.out.println("records below are different! ");
                    System.out.println(urls.get(0) + " : " + srcList.get(j));
                    System.out.println(urls.get(i) + " : " + destList.get(j));
                    throw new ReliabilityException("checkConsistency failed. ");
                }
            }
        }
    }
	
	/*
	 * insert records
	 * @param cl
	 * @param insertNums
	 * @param strLength
	 */
	public static void insertRecords(DBCollection cl,int insertNums,int strLength) {
		BSONObject insertObj = new BasicBSONObject();
		insertObj.put("a", getRandomString(strLength));
		for(int i = 0; i < insertNums; i++) {
			try {
				cl.insert(insertObj);
			}catch (BaseException e) {
				throw e;
			}   	
		}
    }
	
	/*
	 * pop records
	 * @param cl
	 * @param logicalID
	 * @param direction
	 */
	public static void pop(DBCollection cl,long logicalID,int direction) {
		BSONObject popObj = new BasicBSONObject();
		popObj.put("LogicalID", logicalID);
		popObj.put("Direction", direction);
		try {
			cl.pop(popObj);
		}catch (BaseException e) {
			throw e;
		}   	
    }
	
	/*
	 * get one logicalID in capped CL
	 * @param cl
	 * @param skip
	 */
	public static long getLogicalID(DBCollection cl,long skip) {
		long logicalID = -1;
		BSONObject orderBy = new BasicBSONObject();
		orderBy.put("_id", 1);
		DBCursor query = null;
		try {
			long returnOne = 1;
			query = cl.query(null,null,orderBy,null,skip,returnOne);
			while(query.hasNext()) {
				logicalID = (long)query.getNext().get("_id");
			}
		}catch (BaseException e) {
			throw e;
		}finally {
			if(query != null) {
				query.close();
			}
			System.out.println("logicalID: " + logicalID);
		} 
		return logicalID;
    }
	
	/*
	 * get random string for records
	 * @param length
	 */
	public static String getRandomString(int length) {
		Random random = new Random();
        String baseStr = "abcdefghijklmnopqrstuvwxyz";
        int baseLength = baseStr.length();
        StringBuffer strBuf = new StringBuffer();
        for (int i = 0; i < length; ++i) {
            int index = random.nextInt(baseLength);
            strBuf.append(baseStr.charAt(index));
        }
        return strBuf.toString();
    }

    public static void printBeginTime(Object object) {
        System.out.println(object.getClass().getName() + " begin at:"
                + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
    }

    public static void printEndTime(Object object) {
        System.out.println(object.getClass().getName() + " end at:"
                + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
    }

    public static String getKeyStack(Exception e,Object object) {
        StringBuffer stackBuffer = new StringBuffer();
        StackTraceElement[] stackElements = e.getStackTrace();
        for (int i = 0; i < stackElements.length; i++) {
            if (stackElements[i].toString().contains(object.getClass().getName())) {
                stackBuffer.append(stackElements[i].toString()).append("\r\n");
            }
        }
        String str = stackBuffer.toString();
        if (str.length() >= 2) {
            return str.substring(0, str.length() - 2);
        } else {
            return str;
        }

    }
}
