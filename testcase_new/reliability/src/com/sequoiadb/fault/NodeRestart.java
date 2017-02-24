/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:NodeRestart.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.fault ;

import com.sequoiadb.commlib.NodeWrapper ;

public class NodeRestart extends Fault {
    private NodeWrapper node ;

    public NodeRestart( NodeWrapper node ) {
        super( "nodeRestart" ) ;
        // TODO Auto-generated constructor stub

        this.node = node ;

    }

    public void make() {
        this.node.stop() ;
    }

    public boolean checkMakeResult() {
        return this.node.isNodeActive() != true ;
    }

    public void restore() {
        this.node.start() ;
    }

    public boolean checkRestoreResult() {
        return this.node.isNodeActive() == true ;
    }
}
