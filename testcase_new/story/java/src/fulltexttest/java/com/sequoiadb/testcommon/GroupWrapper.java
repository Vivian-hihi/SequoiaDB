package com.sequoiadb.testcommon;

import com.sequoiadb.base.*;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;

import java.util.*;

public class GroupWrapper {

	private ReplicaGroup group;
	private List<NodeWrapper> nodes = new ArrayList<NodeWrapper>();
	private BasicBSONObject groupInfo;
	private GroupMgr mgr;

	public GroupWrapper(BasicBSONObject groupInfo, ReplicaGroup group, GroupMgr mgr) {
		this.groupInfo = groupInfo;
		this.group = group;
		this.mgr = mgr;
	}

	private static final String CATA_RG_NAME = "SYSCatalogGroup";
	private static final String SYSCAT = "SYSCAT";
	private static final String SYSCOLLECTIONS = "SYSCOLLECTIONS";
	private static final String SYSCOLLECTIONSPACES = "SYSCOLLECTIONSPACES";
	private static final String SYSDOMAINS = "SYSDOMAINS";
	private static final String SYSNODES = "SYSNODES";

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
			} catch (InterruptedException e) {

			}
		}
		if (this.getGroupName().equals(CATA_RG_NAME)) {
			return inspectCata(true);
		} else {
			System.out.println(getInspectStdout());
			return false;
		}
	}

	public void refresh(String coordUrl) throws ReliabilityException {
		mgr.refresh(coordUrl);
		this.group = mgr.getGroupByName(getGroupName()).getGroup();
		this.groupInfo = mgr.getGroupByName(getGroupName()).getGroupInfo();
		init();
	}

	public void refresh() throws ReliabilityException {
		refresh(SdbTestBase.coordUrl);
	}

	public void init() throws ReliabilityException {
		String hostName = null;
		String port = null;
		try {
			BasicBSONList nodesinfo = (BasicBSONList) groupInfo.get("Group");
			for (int i = 0; i < nodesinfo.size(); ++i) {
				BasicBSONObject nodeinfo = (BasicBSONObject) nodesinfo.get(i);
				hostName = nodeinfo.getString("HostName");
				port = ((BasicBSONObject) ((BasicBSONList) nodeinfo.get("Service")).get(0)).getString("Name");

				NodeWrapper node = new NodeWrapper(this.group.getNode(hostName, Integer.parseInt(port)), nodeinfo);
				nodes.add(node);
			}
		} catch (BaseException e) {
			System.out.println("hostName:" + hostName + " error:" + e.getErrorCode());
			System.out.println("port:" + port);
			throw new ReliabilityException(e);
		}
	}

	public int getGroupID() {
		return this.groupInfo.getInt("GroupID");
	}

	public String getGroupName() {
		return this.groupInfo.getString("GroupName");
	}

	public ReplicaGroup getGroup() {
		return group;
	}

	public BasicBSONObject getGroupInfo() {
		return groupInfo;
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
				CollectionSpace cs = db.getCollectionSpace(SYSCAT);
				DBCollection cl = cs.getCollection(clName);
				if (cl == null) {
					if (printIncompatibility) {
						System.out.println(SYSCAT + "." + clName + " not exists,host:" + url);
					}
					return false;
				}
				DBCursor cursor = cl.query(null, null, "{_id:1}", null);
				while (cursor.hasNext()) {
					tmp.add(cursor.getNext());
				}
			} catch (BaseException e) {
				if (e.getErrorCode() == -23 || e.getErrorCode() == -34) {
					if (printIncompatibility) {
						System.out.println("SYSCAT or SYSCAT." + clName + " not exists,host:" + url
								+ ", stack on console out put");
						throw e;
					}
					return false;
				} else {
					throw e;
				}
			} finally {
				db.closeAllCursors();
				db.close();
			}
			res.put(url + ":" + clName, tmp);
		}
		Map.Entry<String, List<BSONObject>> tmp2 = null;
		String forPrint = "";
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

	public List<String> getAllUrls() {
		List<String> urls = new ArrayList<String>();
		for (NodeWrapper node : nodes) {
			urls.add(node.hostName() + ":" + node.svcName());
		}
		return urls;
	}

	public String getInspectStdout() throws ReliabilityException {
		Ssh ssh = new Ssh(SdbTestBase.hostName, SdbTestBase.remoteUser, SdbTestBase.remotePwd);
		try {
			ssh.exec(ssh.getSdbInstallDir() + "/bin/sdbinspect -g " + getGroupName());
		} finally {
			ssh.disconnect();
		}
		return ssh.getStdout();
	}

}
