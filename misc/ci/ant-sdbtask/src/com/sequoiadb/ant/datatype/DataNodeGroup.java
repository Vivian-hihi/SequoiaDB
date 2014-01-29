/**
 * 
 */
package com.sequoiadb.ant.datatype;

import org.apache.tools.ant.BuildException;

import com.sequoiadb.base.Shard;
import com.sequoiadb.base.Node;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;

/**
 * @author qiushanggao
 * 
 */
public class DataNodeGroup extends NodeGroup {

	@Override
	public void start(Sequoiadb sdb) throws BuildException {
		try {
			Shard group = sdb.getShard(getName());

			if (group == null) {
				
				group = sdb.createShard(getName());
			}

			for (Node nodeInfo : getNodeList()) {
				Node node = group.getNode(nodeInfo.getHost(),
						nodeInfo.getBasePort());

				if (node == null) {
					System.out.println("host :"+nodeInfo.getHost()+"\n" +
							"port : " +nodeInfo.getBasePort() +"\n" +
							"path : " + nodeInfo.getDbpath() + "\n" +
							"configMap :"+nodeInfo.getConfigMap().toString()
							);
					
					group.createNode(nodeInfo.getHost(),
							nodeInfo.getBasePort(), nodeInfo.getDbpath(),
							nodeInfo.getConfigMap());
				} else {
					throw new BuildException("Node repeat: hostname="
							+ nodeInfo.getHost() + "servicename:"
							+ nodeInfo.getBasePort());
				}
			}

			group.start();

		} catch (Exception e) {

			e.printStackTrace();

			throw new BuildException(e.toString());
		}
	}

	public void waitForStart(Sequoiadb sdb, long timeout) throws BuildException {

		Shard group = sdb.getShard(getName());

		// Wait for group select master, max wait time is 120sec;
		int i = 0;
		while (true) {
			try {
				Node masterNode = group.getMaster();
				if (masterNode != null) {
					break;
				}
			} catch (BaseException baseException) {
			} 
			
			i++;
			if (i >= timeout) {
				throw new BuildException("Group:" + this.getName()
						+ " select master timeout.");
			}
			
			try
			{
				Thread.sleep(1000);
			}
			catch (InterruptedException e) {
			}
		}

		

	}

}
