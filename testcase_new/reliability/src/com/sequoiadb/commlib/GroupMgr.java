/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:GroupMgr.java
 * 类的详细描述
 *
 *  @author wenjingwang
 * Date:2017-2-23上午10:19:55
 *  @version 1.00
 */
package com.sequoiadb.commlib;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;

public class GroupMgr {
	private Map<String, GroupWrapper> name2group = new HashMap<String, GroupWrapper>();
	private Map<Integer, GroupWrapper> id2group = new HashMap<Integer, GroupWrapper>();
	private Sequoiadb sdb = null;
	private static GroupMgr mgr = null;

	private GroupMgr() {
		this.sdb = new Sequoiadb("192.168.31.31:11810", "", "");
	}

	public void init() throws ReliabilityException {
		try {
			BSONObject nullObj = null;
			DBCursor cursor = sdb.getList(Sequoiadb.SDB_LIST_GROUPS, nullObj, nullObj, nullObj);
			while (cursor.hasNext()) {
				BasicBSONObject obj = (BasicBSONObject) cursor.getNext();

				String groupName = obj.getString("GroupName");

				GroupWrapper group = new GroupWrapper(obj, sdb.getReplicaGroup(groupName));
				group.init();
				name2group.put(groupName, group);
				id2group.put(group.getGroupID(), group);
			}
			cursor.close();
		} catch (BaseException e) {
			throw new ReliabilityException(e);
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

	public Set<String> getAllHosts() {
		Set<String> hosts = new HashSet<String>();
		for (Entry<String, GroupWrapper> entry : name2group.entrySet()) {
			Set<String> hostsPerGroup = entry.getValue().getAllHosts();
			hosts.addAll(hostsPerGroup);
		}

		return hosts;
	}

	public GroupWrapper getGroupByName(String name) {
		if (name2group.containsKey(name)) {
			return name2group.get(name);
		} else {
			return null;
		}
	}

	public GroupWrapper getGroupById(int id) {
		if (id2group.containsKey(id)) {
			return id2group.get(id);
		} else {
			return null;
		}
	}

	public static GroupMgr getInstance() throws ReliabilityException {
		mgr = new GroupMgr();
		mgr.init();
		return mgr;
	}

	public boolean checkBusiness() throws ReliabilityException {
		ArrayList<GroupCheckResult> results = new ArrayList<GroupCheckResult>();
		for (Entry<String, GroupWrapper> entry : name2group.entrySet()) {
			if (!entry.getKey().equals("SYSCoord")) {
				results.add(entry.getValue().checkBusiness());
			}
		}

		boolean ret = true;
		for (GroupCheckResult result : results) {
			ret = result.check();
			if (ret == false) {
				System.out.println(result.toString());
			}
		}
		return ret;
	}

	public boolean checkBusinessWithLSN() throws ReliabilityException {
		ArrayList<GroupCheckResult> results = new ArrayList<GroupCheckResult>();
		for (Entry<String, GroupWrapper> entry : name2group.entrySet()) {
			if (!entry.getKey().equals("SYSCoord")) {
				results.add(entry.getValue().checkBusiness());
			}
		}

		boolean ret = true;
		for (GroupCheckResult result : results) {
			ret = result.checkWithLSN();
			if (ret == false) {
				System.out.println(result.toString());
			}
		}
		return ret;
	}

	public boolean checkBusinessWithLSNAndDisk() throws ReliabilityException {
		ArrayList<GroupCheckResult> results = new ArrayList<GroupCheckResult>();
		for (Entry<String, GroupWrapper> entry : name2group.entrySet()) {
			if (!entry.getKey().equals("SYSCoord")) {
				results.add(entry.getValue().checkBusiness());
			}
		}

		boolean ret = true;
		for (GroupCheckResult result : results) {
			ret = result.checkWithLSNAndDiskThreshold();
			if (ret == false) {
				System.out.println(result.toString());
			}
		}
		return ret;
	}

	public boolean checkResidu() {
		boolean checkRet = true;
		Set<String> hosts = getAllHosts();
		for (String host : hosts) {
			try {
				Ssh remote = new Ssh(host, "root", SdbTestBase.rootPwd);
				remote.scpTo("./script/checkPortOccupied.sh", SdbTestBase.workDir);
				remote.scpTo("./script/checkPortOccupied.sh", SdbTestBase.workDir);
				remote.scpTo("./script/checkPortOccupied.sh", SdbTestBase.workDir);

				remote.exec(SdbTestBase.workDir + "/checkPortOccupied.sh");
				if (remote.getExitStatus() != 0) {
					System.out.println(String.format("%s used port:%s", host, remote.getStdout()));
					checkRet = false;
				}
				remote.exec(SdbTestBase.workDir + "/checkPortOccupied.sh");
				if (remote.getExitStatus() != 0) {
					System.out.println(String.format("%s residu config:%s", host, remote.getStdout()));
					checkRet = false;
				}
				remote.exec(SdbTestBase.workDir + "/checkPortOccupied.sh");
				if (remote.getExitStatus() != 0) {
					System.out.println(String.format("%s residu data:%s", host, remote.getStdout()));
					checkRet = false;
				}
			} catch (ReliabilityException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				return false;
			} finally {

			}
		}
		return checkRet;
	}

}
