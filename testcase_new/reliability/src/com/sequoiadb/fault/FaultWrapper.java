/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:FaultWrapper.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.fault ;

import java.text.SimpleDateFormat ;
import java.util.ArrayList ;
import java.util.Calendar ;
import java.util.Date ;
import java.util.List ;

import com.sequoiadb.exception.ReliabilityException ;
import com.sequoiadb.task.OperateTask ;

public class FaultWrapper extends Fault {
    private Fault instance ;
    private int checkTimes = 3 ;
    private List< OperateTask > taskSet = new ArrayList< OperateTask >() ;
    OperateTask.faultStatus status ;

    private void handleException( Exception e ) {
        status = OperateTask.faultStatus.EXCEPTION ;
    }

    public FaultWrapper( Fault instance ) {
        super( instance.getName() ) ;
        // TODO Auto-generated constructor stub
        this.instance = instance ;
    }

    public FaultWrapper( Fault instance, int checkTimes ) {
        super( instance.getName() ) ;
        // TODO Auto-generated constructor stub
        this.instance = instance ;
        this.checkTimes = checkTimes ;
    }

    public void addDependsTask( OperateTask task ) {
        taskSet.add( task ) ;
    }

    public void make() throws ReliabilityException{
        Date date = Calendar.getInstance().getTime() ;
        SimpleDateFormat sdf = new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" ) ;
        System.out.println( "make " + instance.getName() + "at"
                + sdf.format( date ) ) ;

        try {
            instance.make() ;
        } catch ( Exception e ) {
            handleException( e ) ;
        }

    }

    public boolean checkMakeResult() throws ReliabilityException{
        boolean checkResult = false ;
        if ( status != OperateTask.faultStatus.EXCEPTION ) {
            status = OperateTask.faultStatus.MAKEFAILURE ;
            for ( int i = 0; i < checkTimes; ++i ) {
                checkResult = instance.checkMakeResult() ;
                if ( checkResult ) {
                    status = OperateTask.faultStatus.MAKESUCCESS ;
                    break ;
                }
            }
        }
        // 通知依赖的任务
        for ( OperateTask task : taskSet ) {
            task.faultMakeNotify( status ) ;
        }

        return true ;
    }

    public void restore() throws ReliabilityException {
        Date date = Calendar.getInstance().getTime() ;
        SimpleDateFormat sdf = new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" ) ;
        System.out.println( "restore " + instance.getName() + "at"
                + sdf.format( date ) ) ;
        try {
            instance.restore() ;
        } catch ( Exception e ) {
            handleException( e ) ;
        }

    }

    public boolean checkRestoreResult() throws ReliabilityException{
        boolean checkResult = false ;
        if ( status != OperateTask.faultStatus.EXCEPTION ) {
            OperateTask.faultStatus status = OperateTask.faultStatus.RESTOREFAILURE ;
            for ( int i = 0; i < checkTimes; ++i ) {
                checkResult = instance.checkRestoreResult() ;
                if ( checkResult ) {
                    status = OperateTask.faultStatus.RESTORESUCESS ;
                    break ;
                }
            }
        }
        // 通知依赖的任务
        for ( OperateTask task : taskSet ) {
            task.faultRestoreNotify( status ) ;
        }
        return true ;
    }

}
