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

public abstract class Fault {
    private String name ;

    public Fault( String name ) {
        this.name = name ;
    }
    
    public String getName() {
        return name ;
    }

    public abstract void make() ;

    public abstract boolean checkMakeResult() ;

    public abstract void restore() ;

    public abstract boolean checkRestoreResult() ;
}
