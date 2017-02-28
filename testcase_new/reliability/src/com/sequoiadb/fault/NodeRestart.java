/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:NodeRestart.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.fault;

import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.exception.ReliabilityException;

public class NodeRestart extends Fault {
	private NodeWrapper node;

	public NodeRestart(NodeWrapper node) {
		super("nodeRestart");
		// TODO Auto-generated constructor stub

		this.node = node;

	}

	public void make() throws ReliabilityException {
		this.node.stop();
	}

	public boolean checkMakeResult() throws ReliabilityException {
		return this.node.isNodeActive() != true;
	}

	public void restore() throws ReliabilityException {
		this.node.start();
	}

	public boolean checkRestoreResult() throws ReliabilityException {
		return this.node.isNodeActive() == true;
	}

	@Override
	public boolean init() throws ReliabilityException {
		return true;
	}

	@Override
	public boolean fini() throws ReliabilityException {

		return true;
	}
}
