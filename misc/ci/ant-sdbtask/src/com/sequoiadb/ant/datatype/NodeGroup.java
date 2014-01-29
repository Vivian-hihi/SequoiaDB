/**
 * 
 */
package com.sequoiadb.ant.datatype;

import java.util.ArrayList;
import java.util.List;
import org.apache.tools.ant.BuildException;
import com.sequoiadb.base.Sequoiadb;

/**
 * @author qiushanggao
 *
 */
public abstract class NodeGroup {
	private String name;

	private List<ShardNode> nodes = new ArrayList<ShardNode>();
	
	public abstract void start(Sequoiadb sdb) throws BuildException;
	
	public abstract void waitForStart(Sequoiadb sdb, long timeout) throws BuildException;
	
	
	public void setName(String value)
	{
		name = value;
	}
	
	public String getName()
	{
		return name;
	}
	
	public ShardNode createNode()
	{
		ShardNode node = new ShardNode();
		nodes.add(node);
		
		return node;
	}
	
	public List<ShardNode> getNodeList()
	{
		return nodes;
	}
	
	
}
