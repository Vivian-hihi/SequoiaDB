/**
 * Copyright (c) 2017, SequoiaDB Ltd. File Name:GroupWrapper.java
 * 
 *
 * @author wenjingwang Date:2017-2-21下午4:54:48
 * @version 1.00
 */
package com.sequoiadb.commlib;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.bson.util.JSON;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;

public class GroupWrapper {
    private ReplicaGroup group;
    private List<NodeWrapper> nodes = new ArrayList<NodeWrapper>();
    private BasicBSONObject groupInfo;
    private final String CATA_RG_NAME = "SYSCatalogGroup";
    private final String SYSCAT = "SYSCAT";
    private final String SYSCOLLECTIONS = "SYSCOLLECTIONS";
    private final String SYSCOLLECTIONSPACES = "SYSCOLLECTIONSPACES";
    private final String SYSDOMAINS = "SYSDOMAINS";
    private final String SYSNODES = "SYSNODES";

    public GroupWrapper(BasicBSONObject groupInfo, ReplicaGroup group) {

        this.groupInfo = groupInfo;
        this.group = group;
    }

    public String getGroupName() {
        return this.groupInfo.getString("GroupName");
    }

    public int getGroupID() {
        return this.groupInfo.getInt("GroupID");
    }

    public void refresh(String coordUrl) throws ReliabilityException {
        Sequoiadb sdb = new Sequoiadb(coordUrl, "", "");
        try {
            DBCursor cursor = sdb.getList(Sequoiadb.SDB_LIST_GROUPS,
                    (BSONObject) JSON.parse("{GroupName:'" + getGroupName() + "'}"), null, null);
            while (cursor.hasNext()) {
                this.groupInfo = (BasicBSONObject) cursor.getNext();
            }
            nodes.clear();
            init();
        }
        finally {
            sdb.closeAllCursors();
            sdb.disconnect();
        }
    }

    public void refresh() throws ReliabilityException {
        refresh(SdbTestBase.coordUrl);
    }

    public void init() throws ReliabilityException {
        try {
            BasicBSONList nodesinfo = (BasicBSONList) groupInfo.get("Group");
            for (int i = 0; i < nodesinfo.size(); ++i) {
                BasicBSONObject nodeinfo = (BasicBSONObject) nodesinfo.get(i);
                String hostName = nodeinfo.getString("HostName");
                String port = ((BasicBSONObject) ((BasicBSONList) nodeinfo.get("Service")).get(0))
                        .getString("Name");

                NodeWrapper node = new NodeWrapper(
                        this.group.getNode(hostName, Integer.parseInt(port)), nodeinfo);
                nodes.add(node);
            }
        }
        catch (BaseException e) {
            throw new ReliabilityException(e);
        }
    }

    public NodeWrapper getMaster() throws ReliabilityException {
        for (NodeWrapper node : nodes) {
            if (node.isMaster()) {
                return node;
            }
        }
        return null;
    }

    public NodeWrapper getSlave() throws ReliabilityException {
        Random random = new Random();

        int pos = random.nextInt(nodes.size());
        if (!nodes.get(pos).isMaster()) {
            return nodes.get(pos);
        }
        else if (nodes.size() > 1) {
            return nodes.get((pos + 1) % nodes.size());
        }
        else {
            return null;
        }
    }

    public int getNodeNum() {
        return nodes.size();
    }

    /**
     * 至多尝试10次的切主操作（rg.reelect()），若失败则返回false
     * 
     * @param times
     * @return
     * @throws ReliabilityException
     */
    public boolean changePrimary() throws ReliabilityException {
        return changePrimary(10);
    }

    /**
     * 至多尝试times的切主操作（rg.reelect()），若失败则返回false
     * 
     * @param times
     * @return
     * @throws ReliabilityException
     */
    public boolean changePrimary(int times) throws ReliabilityException {
        if (getGroupName().equals("SYSCoord")) {
            return false;
        }
        String groupName = getGroupName();
        int priNode = getMaster().nodeID();
        Ssh ssh = new Ssh(SdbTestBase.hostName, SdbTestBase.remoteUser, SdbTestBase.remotePwd);
        GroupMgr groupMgr = GroupMgr.getInstance();
        try {
            for (int i = 0; i < times; i++) {

                ssh.exec(ssh.getSdbInstallDir()
                        + "/bin/sdb -s \"var db = new Sdb;var rg = db.getRG('" + groupName
                        + "');rg.reelect();\"");
                if (!groupMgr.checkBusiness(120)) {
                    throw new ReliabilityException(
                            "After execute reelect,check business have an error");
                }
                refresh();
                if (priNode != getMaster().nodeID()) {
                    return true;
                }
            }
        }
        finally {
            ssh.close();
        }
        return false;
    }

    public GroupCheckResult checkBusiness(boolean printRes) throws ReliabilityException {
        refresh();
        GroupCheckResult checkRes = new GroupCheckResult();
        if (getGroupName().equals("SYSCoord")) {
            return checkRes;
        }
        checkRes.groupName = getGroupName();
        checkRes.groupID = getGroupID();
        checkRes.primaryNode = groupInfo.getInt("PrimaryNode");

        for (NodeWrapper node : nodes) {
            NodeCheckResult res = node.checkBusiness(printRes);
            checkRes.addNodeCheckResult(res);
        }
        return checkRes;
    }

    public List<String> getAllHosts() {
        List<String> hosts = new ArrayList<String>();
        for (NodeWrapper node : nodes) {
            hosts.add(node.hostName());
        }
        return hosts;
    }

    public List<String> getAllUrls() {
        List<String> urls = new ArrayList<String>();
        for (NodeWrapper node : nodes) {
            urls.add(node.hostName() + ":" + node.svcName());
        }
        return urls;
    }

    /**
     * 检查组间节点一致性，若在checkTimes次数（每次间隔两秒）内仍未检查通过则返回false，并打印对该组执行sdbinspect的输出
     * 
     * @param checkTimes
     * @return
     * @throws ReliabilityException
     */
    public boolean checkInspect(int checkTimes) throws ReliabilityException {
        return checkInspect(checkTimes, 2);
    }

    /**
     * 检查组间节点一致性，若在checkTimes次数（每次间隔intervelSecond秒）内仍未检查通过则返回false，
     * 并打印对该组执行sdbinspect的输出
     * 
     * @param checkTimes
     * @param intervelSecond
     * @return
     * @throws ReliabilityException
     */
    public boolean checkInspect(int checktime, int intervelSecond) throws ReliabilityException {
        this.refresh();
        for (int i = 0; i < checktime; i++) {
            if (inspect()) {
                return true;
            }
            try {
                Thread.sleep(intervelSecond * 1000);
            }
            catch (InterruptedException e) {

            }
        }
        if (this.getGroupName().equals(CATA_RG_NAME)) {
            return inspectCata(true);
        }
        else {
            System.out.println(getInspectStdout());
            return false;
        }
    }

    private boolean inspect() throws ReliabilityException {
        if (this.getGroupName().equals(CATA_RG_NAME)) {
            return inspectCata(false);
        }
        String stdout = getInspectStdout();
        String[] res = stdout.split("\n");
        if (res.length != 8) {
            return false;
        }
        if (res[7].equals("Reason for exit : exit with no records different")) {
            return true;
        }
        return false;
    }

    private boolean inspectCata(boolean printIncompatibility) {
        boolean clFlag = inspectCataCL(SYSCOLLECTIONS, printIncompatibility);
        boolean csFlag = inspectCataCL(SYSCOLLECTIONSPACES, printIncompatibility);
        boolean domainFlag = inspectCataCL(SYSDOMAINS, printIncompatibility);
        boolean nodeFlag = inspectCataCL(SYSNODES, printIncompatibility);
        return nodeFlag && clFlag && csFlag && domainFlag;
    }

    private boolean inspectCataCL(String clName, boolean printIncompatibility) {
        List<String> urls = this.getAllUrls();
        Map<String, List<BSONObject>> res = new HashMap<String, List<BSONObject>>();
        for (String url : urls) {
            List<BSONObject> tmp = new ArrayList<BSONObject>();
            Sequoiadb db = new Sequoiadb(url, "", "");
            try {
                DBCollection cl = db.getCollectionSpace(SYSCAT).getCollection(clName);
                DBCursor cursor = cl.query(null, null, "{_id:1}", null);
                while (cursor.hasNext()) {
                    tmp.add(cursor.getNext());
                }
            }
            finally {
                db.closeAllCursors();
                db.disconnect();
            }
            res.put(url + ":" + clName, tmp);
        }
        Map.Entry<String, List<BSONObject>> tmp2 = null;
        String forPrint = new String();
        boolean ret = true;
        for (Map.Entry<String, List<BSONObject>> entry : res.entrySet()) {
            if (tmp2 != null && !entry.getValue().equals(tmp2.getValue())) {
                ret = false;
            }
            tmp2 = entry;
            forPrint = forPrint + tmp2.toString() + "\r\n";
        }
        if (!ret && printIncompatibility) {
            System.out.println(clName + " incompatible:\r\n" + forPrint);
        }
        return ret;
    }

    public String getInspectStdout() throws ReliabilityException {
        Ssh ssh = new Ssh(SdbTestBase.hostName, SdbTestBase.remoteUser, SdbTestBase.remotePwd);
        try {
            ssh.exec(ssh.getSdbInstallDir() + "/bin/sdbinspect -g " + getGroupName());
        }
        finally {
            ssh.close();
        }
        return ssh.getStdout();
    }

    public List<NodeWrapper> getNodes() {
        return nodes;
    }
}
