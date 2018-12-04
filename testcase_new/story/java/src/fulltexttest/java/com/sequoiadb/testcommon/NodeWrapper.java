package com.sequoiadb.testcommon;

import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Node;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;

public class NodeWrapper {

	private Node node;
	private BasicBSONObject nodeInfo;

	public NodeWrapper(Node node, BasicBSONObject nodeInfo) {
		this.node = node;
		this.nodeInfo = nodeInfo;
	}

	public String hostName() {
		return nodeInfo.getString("HostName");
	}

	public String svcName() {
		return ((BasicBSONObject) ((BasicBSONList) nodeInfo.get("Service")).get(0)).getString("Name");
	}
}
