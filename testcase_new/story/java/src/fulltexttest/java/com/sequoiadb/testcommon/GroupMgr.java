package com.sequoiadb.testcommon;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

public class GroupMgr {

	private Sequoiadb sdb = null;
	private String coordUrl = null;
	private Map<String, GroupWrapper> name2group = new HashMap<String, GroupWrapper>();
	private Map<Integer, GroupWrapper> id2group = new HashMap<Integer, GroupWrapper>();

	public void refresh(String coordUrl) throws ReliabilityException {
		DBCursor cursor = null;
		try {
			if (sdb != null) {
				sdb.close();
			}
			sdb = new Sequoiadb(coordUrl, "", "");
			cursor = sdb.getList(Sequoiadb.SDB_LIST_GROUPS, null, null, null);
			while (cursor.hasNext()) {
				BasicBSONObject obj = (BasicBSONObject) cursor.getNext();

				String groupName = obj.getString("GroupName");

				GroupWrapper group = new GroupWrapper(obj, sdb.getReplicaGroup(groupName), this);
				group.init();
				name2group.put(groupName, group);
				id2group.put(group.getGroupID(), group);
			}
		} catch (BaseException e) {
			throw new ReliabilityException(e);
		} finally {
			if (cursor != null) {
				cursor.close();
			}
		}
	}

	public GroupWrapper getGroupByName(String name) {
		if (name2group.containsKey(name)) {
			return name2group.get(name);
		} else {
			return null;
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

	public GroupMgr() throws ReliabilityException {
		this.refresh();
	}

	public void refresh() throws ReliabilityException {
		if (coordUrl == null) {
			refresh(SdbTestBase.coordUrl);
		} else {
			refresh(coordUrl);
		}
	}

}
