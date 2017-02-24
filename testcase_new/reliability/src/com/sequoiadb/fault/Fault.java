/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:Fault.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.fault ;

import com.sequoiadb.exception.ReliabilityException ;

public abstract class Fault {
    private String name ;

    public Fault( String name ) {
        this.name = name ;
    }
    
    public String getName() {
        return name ;
    }

    public abstract void make() throws ReliabilityException;

    public abstract boolean checkMakeResult() throws ReliabilityException;

    public abstract void restore() throws ReliabilityException;

    public abstract boolean checkRestoreResult() throws ReliabilityException;
}
