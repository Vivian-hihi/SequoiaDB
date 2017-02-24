/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:ReliabilityBaseException.java
 * 类的详细描述
 *
 *  @author wenjingwang
 * Date:2017-2-24上午10:23:57
 *  @version 1.00
 */
package com.sequoiadb.exception;


public class ReliabilityException extends Exception {
    /**
     * 
     */
    private static final long serialVersionUID = 6219985388657559901L ;

    public enum ExceptionType{
       SSHEXCEPTION,
       DBEXCEPTION,
    };
    
    ExceptionType type;
    
    public void setDBException(){
        this.type = ExceptionType.DBEXCEPTION ;
    }
    
    public void setSSHException(){
        this.type = ExceptionType.SSHEXCEPTION ;
    }
    
    public ReliabilityException(String message){
        super(message) ;
    }
    
    public ReliabilityException(){
        super();
    }
    
    public ReliabilityException(String message, Throwable cause){
        super(message, cause) ;
    }
    
    public ReliabilityException(Throwable cause){
        super(cause);
    }
}
