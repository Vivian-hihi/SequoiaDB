package com.sequoiadb.datasync.brokennetwork.commlib;

import java.util.Arrays;
import java.util.List;
import java.util.Random;

import org.bson.types.ObjectId;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.GroupCheckResult;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.ReliabilityException;

public class Utils {
    /**
     * 框架提供的checkBusiness接口会排斥新建节点，认为是组部署异常。
     * 因此要重新定制一个检查集群的方法。
     * @param timeOutSecond
     * @throws ReliabilityException 
     */
    public static boolean checkBusinessForExNode(GroupMgr groupMgr, int timeOutSecond) throws ReliabilityException {
        String lastCheckInfo = "";
        for (int i = 0; i < timeOutSecond; i++) {
            List<String> groupNames = groupMgr.getAllDataGroupName();
            groupNames.remove("SYSCoord");
            boolean ok = true;
            
            try {
                for (String groupName : groupNames) {
                    GroupWrapper group = groupMgr.getGroupByName(groupName);
                    GroupCheckResult grpRes = group.checkBusiness(false);
                    if (!(grpRes.connCheck 
                            && grpRes.primaryCheck 
                            && grpRes.serviceCheck
                            && grpRes.LSNCheck)) {
                        lastCheckInfo = grpRes.toString();
                        ok = false;
                    }
                }
            } catch (Exception e) {
                ok = false;
            }
            
            if (ok) { return true; }
            
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
            }
        }
        System.out.println(lastCheckInfo);
        return false;
    }
    
    public static void testLob(Sequoiadb db, String clName) throws ReliabilityException {
        DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
        int lobSize = 1 * 1024;
        byte[] lobBytes = new byte[lobSize];
        new Random().nextBytes(lobBytes);
        
        DBLob wLob = cl.createLob();
        wLob.write(lobBytes);
        ObjectId oid = wLob.getID();
        wLob.close();
        
        DBLob rLob = cl.openLob(oid);
        byte[] rLobBytes = new byte[lobSize];
        rLob.read(rLobBytes);
        rLob.close();
        
        if (!Arrays.equals(rLobBytes, lobBytes)) {
            throw new ReliabilityException("lob is different");
        }
    }
    
    public static String getKeyStack(Exception e, Object classObj) {
        StringBuffer stackBuffer = new StringBuffer();
        StackTraceElement[] stackElements = e.getStackTrace();
        for (int i = 0; i < stackElements.length; i++) {
            if (stackElements[i].toString().contains(classObj.getClass().getName())) {
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
