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

import com.sequoiadb.exception.FaultException ;
import com.sequoiadb.exception.ReliabilityException ;
import com.sequoiadb.task.OperateTask ;

public class FaultWrapper extends Fault {
    private Fault instance ;
    private int checkTimes = 3 ;
    private List< OperateTask > taskSet = new ArrayList< OperateTask >() ;
    OperateTask.faultStatus status ;

    private void handleException( FaultException e ) {
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

    public void removeDependsTask( OperateTask task ) {
        int pos = 0 ;
        for ( ; pos < taskSet.size(); ++pos ) {
            if ( task.getName().equals( taskSet.get( pos ).getName() ) ) {
                break ;
            }
        }
        taskSet.remove( pos ) ;
    }

    public void make() throws FaultException {
        FaultException exception = null ;
        Date date = Calendar.getInstance().getTime() ;
        SimpleDateFormat sdf = new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" ) ;
        System.out.println( "make " + instance.getName() + "at"
                + sdf.format( date ) ) ;

        try {
            instance.make() ;
            if ( status != OperateTask.faultStatus.EXCEPTION ) {
                if ( checkMakeResult() ) {
                    date = Calendar.getInstance().getTime() ;
                    sdf = new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" ) ;
                    System.out.println( "make " + instance.getName()
                            + "success at " + sdf.format( date ) ) ;
                }
            }
        } catch ( FaultException e ) {
            handleException( e ) ;
            exception = e ;
        }

        // 通知依赖的任务
        for ( OperateTask task : taskSet ) {
            task.faultMakeNotify( status ) ;
        }

        if ( exception != null ) {
            throw exception ;
        }

    }

    public boolean checkMakeResult() throws FaultException {
        boolean checkResult = false ;
        status = OperateTask.faultStatus.MAKEFAILURE ;
        for ( int i = 0; i < checkTimes; ++i ) {
            checkResult = instance.checkMakeResult() ;
            if ( checkResult ) {
                status = OperateTask.faultStatus.MAKESUCCESS ;
                break ;
            }
        }
        return true ;
    }

    public void restore() throws FaultException {
        FaultException exception = null ;
        if ( status != OperateTask.faultStatus.MAKESUCCESS ) {
            return ;
        }

        Date date = Calendar.getInstance().getTime() ;
        SimpleDateFormat sdf = new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" ) ;
        System.out.println( "restore " + instance.getName() + "at"
                + sdf.format( date ) ) ;
        try {
            instance.restore() ;
            if ( status != OperateTask.faultStatus.EXCEPTION ) {
                if ( checkRestoreResult() ) {
                    ;
                }
            }
        } catch ( FaultException e ) {
            handleException( e ) ;
            exception = e ;
        }

        // 通知依赖的任务
        for ( OperateTask task : taskSet ) {
            task.faultRestoreNotify( status ) ;
        }

        if ( exception != null ) {
            throw exception ;
        }
    }

    public boolean checkRestoreResult() throws FaultException {
        boolean checkResult = false ;
        status = OperateTask.faultStatus.RESTOREFAILURE ;
        for ( int i = 0; i < checkTimes; ++i ) {
            checkResult = instance.checkRestoreResult() ;
            if ( checkResult ) {
                status = OperateTask.faultStatus.RESTORESUCESS ;
                break ;
            }
        }
        return true ;
    }

    @Override
    public boolean init() throws FaultException {
        return instance.init() ;
    }

    @Override
    public boolean fini() throws FaultException {
        return instance.fini() ;
    }

}
