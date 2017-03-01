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

import com.sequoiadb.exception.FaultException ;
import com.sequoiadb.exception.ReliabilityException ;

public class NodeRestart extends Fault {
    private NodeWrapper node ;

    public NodeRestart( NodeWrapper node ) {
        super( "nodeRestart" ) ;
        // TODO Auto-generated constructor stub

        this.node = node ;

    }

    public void make() throws FaultException {
        try {
            this.node.stop() ;
        } catch ( ReliabilityException e ) {
            throw new FaultException( e ) ;
        }
    }

    public boolean checkMakeResult() throws FaultException {
        try {
            return this.node.isNodeActive() != true ;
        } catch ( ReliabilityException e ) {
            throw new FaultException( e ) ;
        }
    }

    public void restore() throws FaultException {
        try {
            this.node.start() ;
        } catch ( ReliabilityException e ) {
            throw new FaultException( e ) ;
        }
    }

    public boolean checkRestoreResult() throws FaultException {
        try {
            return this.node.isNodeActive() == true ;
        } catch ( ReliabilityException e ) {
            throw new FaultException( e ) ;
        }
    }

    @Override
    public boolean init() throws FaultException {
        return true ;
    }

    @Override
    public boolean fini() throws FaultException {
        return true ;
    }
}
