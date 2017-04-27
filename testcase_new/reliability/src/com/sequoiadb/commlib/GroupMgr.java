/**
 * Copyright (c) 2017, SequoiaDB Ltd. File Name:GroupMgr.java 类的详细描述
 *
 * @author wenjingwang Date:2017-2-23上午10:19:55
 * @version 1.00
 */
package com.sequoiadb.commlib;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;

public class GroupMgr {
    private Map<String, GroupWrapper> name2group = new HashMap<String, GroupWrapper>();
    private Map<Integer, GroupWrapper> id2group = new HashMap<Integer, GroupWrapper>();
    private static GroupMgr mgr = null;
    private Sequoiadb sdb = null;
    private String coordUrl = null;
    
    static {
        try {
            mgr = new GroupMgr();
        } catch (ReliabilityException e) {
            e.printStackTrace();
        }
    }

    public GroupMgr() throws ReliabilityException {
        this.refresh();
    }

    public GroupMgr(String coordUrl) throws ReliabilityException {
        this.coordUrl = coordUrl;
        this.refresh(coordUrl);
    }

    public void refresh(String coordUrl) throws ReliabilityException {
        DBCursor cursor = null;
        try {
            if (sdb != null) {
                sdb.close();
            }
            sdb = new Sequoiadb(coordUrl, "", "");
            BSONObject nullObj = null;
            cursor = sdb.getList(Sequoiadb.SDB_LIST_GROUPS, nullObj, nullObj, nullObj);
            while (cursor.hasNext()) {
                BasicBSONObject obj = (BasicBSONObject) cursor.getNext();

                String groupName = obj.getString("GroupName");

                GroupWrapper group = new GroupWrapper(obj, sdb.getReplicaGroup(groupName),this);
                group.init();
                name2group.put(groupName, group);
                id2group.put(group.getGroupID(), group);
            }
        }
        catch (BaseException e) {
            throw new ReliabilityException(e);
        }
        finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    public void refresh() throws ReliabilityException {
        if (coordUrl == null) {
            refresh(SdbTestBase.coordUrl);
        }
        else {
            refresh(coordUrl);
        }
    }

    public List<GroupWrapper> getAllDataGroup() {
        List<GroupWrapper> dataGroups = new ArrayList<GroupWrapper>();
        for (Entry<String, GroupWrapper> entry : name2group.entrySet()) {
            if (!entry.getKey().equals("SYSSpare") && !entry.getKey().equals("SYSCatalogGroup")
                    && !entry.getKey().equals("SYSCoord")) {
                dataGroups.add(entry.getValue());
            }
        }

        return dataGroups;
    }

    public List<String> getAllDataGroupName() {
        List<String> names = new ArrayList<String>();
        for (Entry<String, GroupWrapper> entry : name2group.entrySet()) {
            if (!entry.getKey().equals("SYSSpare") && !entry.getKey().equals("SYSCatalogGroup")
                    && !entry.getKey().equals("SYSCoord")) {
                names.add(entry.getKey());
            }
        }
        return names;
    }

    public List<String> getAllHosts() {
        List<String> hosts = new ArrayList<String>();
        for (Entry<String, GroupWrapper> entry : name2group.entrySet()) {
            List<String> hostsPerGroup = entry.getValue().getAllHosts();
            hosts.addAll(hostsPerGroup);
        }

        return hosts;
    }

    public GroupWrapper getGroupByName(String name) {
        if (name2group.containsKey(name)) {
            return name2group.get(name);
        }
        else {
            return null;
        }
    }

    public GroupWrapper getGroupById(int id) {
        if (id2group.containsKey(id)) {
            return id2group.get(id);
        }
        else {
            return null;
        }
    }

    @Deprecated
    public static GroupMgr getInstance() throws ReliabilityException {
        mgr = new GroupMgr();
        return mgr;
    }

    /**
     * 持续检测集群当前的状态（组内是否有主，各节点可连接，ServiceStatus），若在timeOutSecond时间内检测仍不通过，
     * 则会打印当前集群状态信息（也可能会抛出异常，如-104，网络错误等等），并返回false
     * Assert.assertEquals(checkBusiness(120), true, "Message");
     * 
     * @param timeOutSecond
     *            建议>120（秒）
     * @return
     * @throws ReliabilityException
     */
    public boolean checkBusiness(int timeOutSecond) throws ReliabilityException {
        long timestamp = System.currentTimeMillis();
        while (!mgr.checkBusiness(false)) {
            if (System.currentTimeMillis() - timestamp > timeOutSecond * 1000) {
                return mgr.checkBusiness(true);
            }
            try {
                Thread.sleep(1000);
            }
            catch (InterruptedException e) {
                // ignore
            }
        }
        return true;
    }

    /**
     * 持续检测集群当前的状态120秒（组内是否有主，各节点可连接，ServiceStatus），若在timeOutSecond时间内检测仍不通过，
     * 则会打印当前集群状态信息（也可能会抛出异常，如-104，网络错误等等），并返回false
     * Assert.assertEquals(checkBusiness(), true, "Message");
     * 
     * @return
     * @throws ReliabilityException
     */
    public boolean checkBusiness() throws ReliabilityException {
        return checkBusiness(120);
    }

    /**
     * 检测集群状态（组内是否有主，各节点可连接，ServiceStatus），
     * 失败则根据变量printAndThrowAllException决定是否打印集群信息，是否屏蔽检测过程中发生的异常，并返回false
     * 
     * @param printAndThrowAllException
     * @return
     * @throws ReliabilityException
     */
    // TODO:可被替代，屏蔽
    private boolean checkBusiness(boolean printAndThrowAllException) throws ReliabilityException {
        ArrayList<GroupCheckResult> results = new ArrayList<GroupCheckResult>();
        for (Entry<String, GroupWrapper> entry : name2group.entrySet()) {
            if (!entry.getKey().equals("SYSCoord")) {
                try {
                    results.add(entry.getValue().checkBusiness(printAndThrowAllException));
                }
                catch (Exception e) {
                    if (printAndThrowAllException) {
                        throw e;
                    }
                    return false;
                }
            }
        }

        boolean ret = true;
        for (GroupCheckResult result : results) {
            if (!result.check()) {
                if (printAndThrowAllException) {
                    System.out.println(result.toString());
                }
                ret = false;
            }
        }
        if (ret == true) {
            // 尝试创建一个ReplSize=3的测试集合（检测所有数据节点是否Alive）
            if (createTestCollection(printAndThrowAllException)) {
                // 检查所有编目节点是否可以查询到建立的测试集合（检测所有编目节点是否Alive）
                return testCatalogSync(printAndThrowAllException);
            }
            else {
                return false;
            }
        }
        return ret;
    }

    private boolean testCatalogSync(boolean printAndThrowAllException) throws ReliabilityException {
        GroupWrapper catagroup = new GroupMgr().getGroupByName("SYSCatalogGroup");
        List<NodeWrapper> nodes = catagroup.getNodes();
        boolean ret = true;
        for (NodeWrapper node : nodes) {
            Sequoiadb db = node.connect();
            try {
                DBCollection cl = db.getCollectionSpace("SYSCAT").getCollection("SYSCOLLECTIONS");
                if (cl == null) {
                    if (printAndThrowAllException) {
                        System.out.println(
                                "Check business:failed to query test collection(clForTestBusiness_reliability) on SYSCatalogGroup:Can not find SYSCAT.SYSCOLLECTIONS");
                    }
                    ret = false;
                }
                long count = cl.getCount(
                        "{Name:'" + SdbTestBase.csName + ".clForTestBusiness_reliability'}");
                if (count == 0) {
                    if (printAndThrowAllException) {
                        System.out.println(
                                "Check business:failed to query test collection(clForTestBusiness_reliability) on SYSCatalogGroup:Can not find clForTestBusiness_reliability");
                    }
                    ret = false;
                }
            }
            catch (BaseException e) {
                if (printAndThrowAllException) {
                    System.out.println(
                            "Check business:failed to query test collection(clForTestBusiness_reliability) on SYSCatalogGroup:"
                                    + e.getErrorCode());
                    throw new ReliabilityException(e);
                }
                ret = false;
            }
            finally {
                db.close();
            }
        }
        Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        try {
            db.getCollectionSpace(SdbTestBase.csName)
                    .dropCollection("clForTestBusiness_reliability");
        }
        catch (BaseException e) {
            System.out.println("Check business:failed to drop test collection:" + e.getErrorCode());
            throw new ReliabilityException(e);
        }
        finally {
            db.close();
        }
        return ret;
    }

    private boolean createTestCollection(boolean printAndThrowAllException)
            throws ReliabilityException {
        Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        CollectionSpace cs = db.getCollectionSpace(SdbTestBase.csName);
        List<String> groupNames = new GroupMgr().getAllDataGroupName();
        int index = 0;
        try {
            for (index = 0; index < groupNames.size(); index++) {
                cs.createCollection("clForTestBusiness_reliability", (BSONObject) JSON
                        .parse("{ReplSize:3,Group:'" + groupNames.get(index) + "'}"));

                if (index != groupNames.size() - 1) {
                    cs.dropCollection("clForTestBusiness_reliability");
                }
            }

        }
        catch (BaseException e) {
            if (printAndThrowAllException) {
                System.out.println(
                        "Check business:failed to create test collection(clForTestBusiness_reliability) on "
                                + groupNames.get(index) + ":" + e.getErrorCode());
                throw new ReliabilityException(e);
            }
            return false;
        }
        finally {
            db.close();
        }
        return true;
    }

    /**
     * 持续检测集群当前的状态（组内是否有主，各节点可连接，ServiceStatus,LSN一致），若在timeOutSecond时间内检测仍不通过，
     * 则会打印当前集群状态信息（也可能会抛出异常，如-104，网络错误等等），并返回false
     * Assert.assertEquals(checkBusiness(120), true, "Message");
     * 
     * @param timeOutSecond
     *            建议>120（秒）
     * @return
     * @throws ReliabilityException
     */
    public boolean checkBusinessWithLSN(int timeOutSecond) throws ReliabilityException {
        long timestamp = System.currentTimeMillis();
        while (!mgr.checkBusinessWithLSN(false)) {
            if (System.currentTimeMillis() - timestamp > timeOutSecond * 1000) {
                return mgr.checkBusinessWithLSN();
            }
            try {
                Thread.sleep(1000);
            }
            catch (InterruptedException e) {
                // ignore
            }
        }
        return true;
    }

    /**
     * 检测集群状态（组内是否有主，各节点可连接，ServiceStatus,组内LSN一致），失败则打印集群信息，并返回false
     * 
     * @return
     * @throws ReliabilityException
     */
    // TODO:可被替代，屏蔽
    private boolean checkBusinessWithLSN() throws ReliabilityException {
        return checkBusinessWithLSN(true);
    }

    /**
     * 检测集群状态（组内是否有主，各节点可连接，ServiceStatus,组内LSN一致），
     * 失败则根据变量printAndThrowAllException决定是否打印集群信息，是否屏蔽检测过程中发生的异常，并返回false
     * 
     * @param printAndThrowAllException
     * @return
     * @throws ReliabilityException
     */
    // TODO:可被替代，屏蔽
    private boolean checkBusinessWithLSN(boolean printAndThrowAllException)
            throws ReliabilityException {
        ArrayList<GroupCheckResult> results = new ArrayList<GroupCheckResult>();
        for (Entry<String, GroupWrapper> entry : name2group.entrySet()) {
            if (!entry.getKey().equals("SYSCoord")) {
                try {
                    results.add(entry.getValue().checkBusiness(printAndThrowAllException));
                }
                catch (Exception e) {
                    if (printAndThrowAllException) {
                        throw e;
                    }
                    return false;
                }
            }
        }

        boolean ret = true;
        for (GroupCheckResult result : results) {
            if (!result.checkWithLSN()) {
                if (printAndThrowAllException) {
                    System.out.println(result.toString());
                }
                ret = false;
            }
        }

        if (ret == true) {
            // 尝试创建一个ReplSize=3的测试集合（检测所有数据节点是否Alive）
            if (createTestCollection(printAndThrowAllException)) {
                // 检查所有编目节点是否可以查询到建立的测试集合（检测所有编目节点是否Alive）
                return testCatalogSync(printAndThrowAllException);
            }
        }
        return ret;
    }

    /**
     * 持续检测集群当前的状态（组内是否有主，各节点可连接，ServiceStatus,LSN一致，节点所在磁盘剩余空间大于128M），
     * 若在timeOutSecond时间内检测仍不通过， 则会打印当前集群状态信息（也可能会抛出异常，如-104，网络错误等等），并返回false
     * Assert.assertEquals(checkBusiness(120), true, "Message");
     * 
     * @param timeOutSecond
     *            建议>120（秒）
     * @return
     * @throws ReliabilityException
     */
    public boolean checkBusinessWithLSNAndDisk(int timeOutSecond) throws ReliabilityException {
        long timestamp = System.currentTimeMillis();
        while (!mgr.checkBusinessWithLSNAndDisk(false)) {
            if (System.currentTimeMillis() - timestamp > timeOutSecond * 1000) {
                return mgr.checkBusinessWithLSNAndDisk();
            }
            try {
                Thread.sleep(1000);
            }
            catch (InterruptedException e) {
                // ignore
            }
        }
        return true;
    }

    /**
     * 检测集群状态（组内是否有主，各节点可连接，ServiceStatus,组内LSN一致,节点所在磁盘剩余空间大于128M），失败则打印集群信息，
     * 并返回false
     * 
     * @return
     * @throws ReliabilityException
     */
    // 可被替代，屏蔽
    private boolean checkBusinessWithLSNAndDisk() throws ReliabilityException {
        return checkBusinessWithLSNAndDisk(true);
    }

    /**
     * 检测集群状态（组内有主，各节点可连接，ServiceStatus,组内LSN一致,节点所在磁盘剩余空间大于128M），
     * 失败则根据变量printAndThrowAllException决定是否打印集群信息，是否屏蔽检测过程中发生的异常，并返回false
     * 
     * @param printAndThrowAllException
     * @return
     * @throws ReliabilityException
     */
    // TODO:可被替代，屏蔽
    public boolean checkBusinessWithLSNAndDisk(boolean printAndThrowAllException)
            throws ReliabilityException {
        ArrayList<GroupCheckResult> results = new ArrayList<GroupCheckResult>();
        for (Entry<String, GroupWrapper> entry : name2group.entrySet()) {
            if (!entry.getKey().equals("SYSCoord")) {
                try {
                    results.add(entry.getValue().checkBusiness(printAndThrowAllException));
                }
                catch (Exception e) {
                    if (printAndThrowAllException) {
                        throw e;
                    }
                    return false;
                }
            }
        }

        boolean ret = true;
        for (GroupCheckResult result : results) {
            if (!result.checkWithLSNAndDiskThreshold()) {
                if (printAndThrowAllException) {
                    System.out.println(result.toString());
                }
                ret = false;
            }
        }

        if (ret == true) {
            // 尝试创建一个ReplSize=3的测试集合（检测所有数据节点是否Alive）
            if (createTestCollection(printAndThrowAllException)) {
                // 检查所有编目节点是否可以查询到建立的测试集合（检测所有编目节点是否Alive）
                return testCatalogSync(printAndThrowAllException);
            }
        }
        return ret;
    }

    /**
     * 检测端口SdbTestBase.reservedPortBegin-SdbTestBase.reservedPortEnd是否有残留占用
     * 检测目录SdbTestBase.reservedDir下是否有数据文件残留,检测安装目录下是否有用例节点配置文件残留
     * 以上检测针对集群所有主机，检测不通过返回false，并打印检测信息
     * 
     * @return
     */
    public boolean checkResidu() {
        boolean checkRet = true;
        List<String> hosts = getAllHosts();
        for (String host : hosts) {
            try {
                Ssh remote = new Ssh(host, "root", SdbTestBase.rootPwd);
                remote.scpTo("./script/checkCfgResidu.sh", SdbTestBase.workDir);
                remote.exec("chmod 777 " + SdbTestBase.workDir + "/checkCfgResidu.sh");
                remote.scpTo("./script/checkPortOccupied.sh", SdbTestBase.workDir);
                remote.exec("chmod 777 " + SdbTestBase.workDir + "/checkPortOccupied.sh");
                remote.scpTo("./script/checkDataResidu.sh", SdbTestBase.workDir);
                remote.exec("chmod 777 " + SdbTestBase.workDir + "/checkDataResidu.sh");

                try {
                    remote.exec(SdbTestBase.workDir + "/checkPortOccupied.sh "
                            + SdbTestBase.reservedPortBegin + " " + SdbTestBase.reservedPortEnd);
                }
                catch (ReliabilityException e) {
                    System.out.println(String.format("%s used port:%s", host, remote.getStdout()));
                    if (remote.getStderr().length() != 0) {
                        System.out.println("StdErr:" + remote.getStderr());
                    }
                    checkRet = false;
                }
               
                try {
                    remote.exec(SdbTestBase.workDir + "/checkCfgResidu.sh "
                            + SdbTestBase.reservedPortBegin + " " + SdbTestBase.reservedPortEnd);
                }
                catch (ReliabilityException e) {
                    System.out.println(
                            String.format("%s residu config:%s", host, remote.getStdout()));
                    if (remote.getStderr().length() != 0) {
                        System.out.println("StdErr:" + remote.getStderr());
                    }
                    checkRet = false;
                }
                
                try {
                    remote.exec(
                            SdbTestBase.workDir + "/checkDataResidu.sh " + SdbTestBase.reservedDir);
                }
                catch (ReliabilityException e) {
                    System.out
                            .println(String.format("%s residu data:%s", host, remote.getStdout()));
                    if (remote.getStderr().length() != 0) {
                        System.out.println("StdErr:" + remote.getStderr());
                    }
                    checkRet = false;
                }
            }
            catch (ReliabilityException e) {
                e.printStackTrace();
                return false;
            }
            finally {

            }
        }
        return checkRet;
    }

    public void close() {
        if (sdb != null) {
            sdb.close();
        }
    }

}
