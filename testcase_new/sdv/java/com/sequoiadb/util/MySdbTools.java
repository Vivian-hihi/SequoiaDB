package com.sequoiadb.util;

import java.util.ArrayList;

import org.bson.BSONObject;
import org.bson.types.BasicBSONList;
import org.bson.util.JSON;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Domain;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;

public class MySdbTools {

	public static final int FLG_INSERT_CONTONDUP = 0x00000001;

	public static CollectionSpace createCS(String csName, Sequoiadb db) throws Exception {
		if (db == null) {
			throw new Exception("Sequoiadb is null,can not create " + csName + " CollectionSpace!");
		}
		CollectionSpace tmp = null;
		try {
			if (db.isCollectionSpaceExist(csName)) {
				db.dropCollectionSpace(csName);
			}
			tmp = db.createCollectionSpace(csName);

		} catch (BaseException e) {
			throw e;
		}
		return tmp;
	}

	public static CollectionSpace createCS(String csName, Sequoiadb db, String option) throws Exception {
		if (db == null) {
			throw new Exception("Sequoiadb is null,can not create " + csName + " CollectionSpace!");
		}
		CollectionSpace tmp = null;
		try {
			if (db.isCollectionSpaceExist(csName)) {
				db.dropCollectionSpace(csName);
			}
			tmp = db.createCollectionSpace(csName, (BSONObject) JSON.parse(option));

		} catch (BaseException e) {
			throw e;
		}
		return tmp;
	}

	public static Domain createDomain(Sequoiadb sdb, String name, ArrayList<String> groupArr, int size,
			boolean autoSplit) throws Exception {
		Domain domain = null;
		if (sdb == null) {
			throw new Exception("Sequoiadb is null,can not create " + name + " Domain!");
		}
		try {
			if (sdb.isDomainExist(name)) {
				domain = sdb.getDomain(name);
			} else {
				StringBuilder groups = new StringBuilder();
				String option = new String();
				for (int i = 0; i < groupArr.size() && i < size; i++) {
					groups.append("\"").append(groupArr.get(i)).append("\",");
				}
				groups.deleteCharAt(groups.length() - 1);
				groups.insert(0, "[");
				groups.append("]");
				if (autoSplit) {
					option = "{\"Groups\":" + groups + ",\"AutoSplit\":true}";
				} else {
					option = "{\"Groups\":" + groups + ",\"AutoSplit\":false}";
				}
				domain = sdb.createDomain(name, (BSONObject) JSON.parse(option));
			}

		} catch (Exception e) {
			throw e;
		}
		return domain;
	}

	public static DBCollection createCL(String clName, CollectionSpace cs, String option) throws Exception {
		if (cs == null) {
			throw new Exception("CollectionSpace is null,can not create " + clName + " Collection!");
		}
		DBCollection tmp = null;
		try {
			if (cs.isCollectionExist(clName)) {
				cs.dropCollection(clName);
			}
			tmp = cs.createCollection(clName, (BSONObject) JSON.parse(option));
		} catch (BaseException e) {
			throw e;
		}
		return tmp;
	}

	public static ArrayList<String> getGroupName(Sequoiadb sdb, String csName, String clName) throws Exception {
		if (sdb == null) {
			throw new Exception("Sequoiadb is null,can not get GroupName");
		}
		DBCursor dbc = null;
		;
		ArrayList<String> resault = new ArrayList<>();
		try {
			CommLib commlib = new CommLib();
			ArrayList<String> groups = commlib.getDataGroupNames(sdb);
			dbc = sdb.getSnapshot(Sequoiadb.SDB_SNAP_CATALOG, "{Name:\"" + csName + "." + clName + "\"}", null, null);
			BasicBSONList list = null;
			if (dbc.hasNext()) {
				list = (BasicBSONList) dbc.getNext().get("CataInfo");
			} else {
				throw new Exception("Sequoiadb can not find Collection:"+csName+"."+clName);
			}
			String srcGroupName = (String) ((BSONObject) list.get(0)).get("GroupName");
			resault.add(srcGroupName);
			if (groups.size() < 2) {
				return resault;
			}
			String destGroupName;
			if (srcGroupName.equals(groups.get(0)))
				destGroupName = groups.get(1);
			else
				destGroupName = groups.get(0);
			resault.add(destGroupName);
			return resault;
		} catch (BaseException e) {
			throw new Exception(e.getMessage());
		} finally {
			if (dbc != null) {
				dbc.close();
			}
		}
	}

}
