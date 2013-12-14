package com.sequoiadb.ant.datatype;

import java.util.ArrayList;
import java.util.List;

import org.apache.tools.ant.BuildException;

import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.ReplicaNode;
import com.sequoiadb.base.Sequoiadb;

public abstract class HostName extends NodeGroup{
	private List<String> hostName = new ArrayList<String>();
	private List<Node> nodes = new ArrayList<Node>();
	public void start(Sequoiadb sdb) throws BuildException {
	try {
		ReplicaGroup group = sdb.getReplicaGroup(getName());
		
		
		for (Node nodeInfo : getNodeList()) {
			hostName.add(group.getNode(nodeInfo.getHost()).toString());
		}

	} catch (Exception e) {

		e.printStackTrace();

		throw new BuildException(e.toString());
	}
}
}
