/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:FaultException.java
 * 类的详细描述
 *
 *  @author 类创建者姓名
 * Date:2017-2-24上午10:43:42
 *  @version 1.00
 */
package com.sequoiadb.exception ;

import com.jcraft.jsch.JSchException ;

public class FaultException extends ReliabilityException {
    /**
     * 
     */
    private static final long serialVersionUID = -218223203783908559L ;

    public FaultException( String message ) {
        super( message ) ;
    }

    public FaultException() {
        super() ;
    }

    public FaultException( String message, Throwable cause ) {
        super( message, cause ) ;

        if ( cause instanceof BaseException ) {
            super.setDBException() ;
        } else if ( cause instanceof JSchException ) {
            super.setSSHException() ;
        }
    }

    public FaultException( Throwable cause ) {
        super( cause ) ;
    }
}
