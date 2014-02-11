package com.sequoiadb.ant.datatype;

import java.util.ArrayList;
import java.util.List;

import org.apache.tools.ant.BuildException;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.base.Shard;

public abstract class HostName extends NodeGroup{
	private List<String> hostName = new ArrayList<String>();
	private List<Node> nodes = new ArrayList<Node>();
	public void start(Sequoiadb sdb) throws BuildException {
	try {
		Shard group = sdb.getShard(getName());
		
		
		for (Node nodeInfo : getNodeList()) {
			hostName.add(group.getNode(nodeInfo.getHost()).toString());
		}

	} catch (Exception e) {

		e.printStackTrace();

		throw new BuildException(e.toString());
	}
}
}
