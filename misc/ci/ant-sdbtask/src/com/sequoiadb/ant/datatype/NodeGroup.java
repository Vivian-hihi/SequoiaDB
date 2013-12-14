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

	private List<Node> nodes = new ArrayList<Node>();
	
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
	
	public Node createNode()
	{
		Node node = new Node();
		nodes.add(node);
		
		return node;
	}
	
	public List<Node> getNodeList()
	{
		return nodes;
	}
	
	
}
