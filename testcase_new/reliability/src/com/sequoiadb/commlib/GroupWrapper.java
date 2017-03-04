/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:GroupWrapper.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.commlib;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Random;
import java.util.Set;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.bson.util.JSON;

import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;

public class GroupWrapper {
	private ReplicaGroup group;
	private List<NodeWrapper> nodes = new ArrayList<NodeWrapper>();
	private BasicBSONObject groupInfo;

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

	public void refresh() throws ReliabilityException {
		Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		DBCursor cursor = sdb.getList(Sequoiadb.SDB_LIST_GROUPS,
				(BSONObject) JSON.parse("{GroupName:'" + getGroupName() + "'}"), null, null);
		while (cursor.hasNext()) {
			this.groupInfo = (BasicBSONObject) cursor.getNext();
		}
		init();
		cursor.close();
		sdb.disconnect();
	}

	public void init() throws ReliabilityException {
		try {
			BasicBSONList nodesinfo = (BasicBSONList) groupInfo.get("Group");
			for (int i = 0; i < nodesinfo.size(); ++i) {
				BasicBSONObject nodeinfo = (BasicBSONObject) nodesinfo.get(i);
				String hostName = nodeinfo.getString("HostName");
				String port = ((BasicBSONObject) ((BasicBSONList) nodeinfo.get("Service")).get(0)).getString("Name");

				NodeWrapper node = new NodeWrapper(this.group.getNode(hostName, Integer.parseInt(port)), nodeinfo);
				nodes.add(node);
			}
		} catch (BaseException e) {
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
		} else if (nodes.size() > 1) {
			return nodes.get((pos + 1) % nodes.size());
		} else {
			return null;
		}
	}

	public int getNodeNum() {
		return nodes.size();
	}

	public boolean changePrimary(int times) throws ReliabilityException {
		if (getGroupName().equals("SYSCoord")) {
			return false;
		}
		String groupName = getGroupName();
		int priNode = getMaster().nodeID();
		Ssh ssh = new Ssh(SdbTestBase.hostName, SdbTestBase.remoteUser, SdbTestBase.remotePwd);
		try {
			for (int i = 0; i < times; i++) {
				ssh.exec("sdb -s \"var db = new Sdb;var rg = db.getRG('" + groupName + "');rg.reelect();\"");
				refresh();
				if (priNode != getMaster().nodeID()) {
					return true;
				}
			}
		} finally {
			ssh.close();
		}
		return false;
	}

	public GroupCheckResult checkBusiness() {
		GroupCheckResult checkRes = new GroupCheckResult();
		if (getGroupName().equals("SYSCoord")) {
			return checkRes;
		}
		checkRes.groupName = getGroupName();
		checkRes.groupID = getGroupID();
		checkRes.primaryNode = groupInfo.getInt("PrimaryNode");

		for (NodeWrapper node : nodes) {
			NodeCheckResult res = node.checkBusiness();
			checkRes.addNodeCheckResult(res);
		}

		return checkRes;
	}

	public Set<String> getAllHosts() {
		Set<String> hosts = new HashSet<String>();
		for (NodeWrapper node : nodes) {
			hosts.add(node.hostName());
		}
		return hosts;
	}

	public boolean checkInspect(int checktime, int intervelSecond) throws ReliabilityException {
		for (int i = 0; i < checktime; i++) {
			if (inspect()) {
				return true;
			}
			try {
				Thread.sleep(intervelSecond*1000);
			} catch (InterruptedException e) {

			}
			
		}
		return false;

	}

	private boolean inspect() throws ReliabilityException {
		String stdout = getInspectStdout();
		String[] res = stdout.split("\n");
		if (res.length != 8) {
			throw new ReliabilityException("analyze inspectStdout faile:" + stdout);
		}
		if (res[7].equals("Reason for exit : exit with no records different")) {
			return true;
		}
		return false;
	}

	public String getInspectStdout() throws ReliabilityException {
		Ssh ssh = new Ssh(SdbTestBase.hostName, SdbTestBase.remoteUser, SdbTestBase.remotePwd);
		ssh.exec("sdbinspect -g " + getGroupName());
		return ssh.getStdout();
	}
}
