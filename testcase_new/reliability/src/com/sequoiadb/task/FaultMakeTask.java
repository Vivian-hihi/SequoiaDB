/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:FaultMakeTask.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.task ;

import java.util.List ;
import java.util.Random ;

import com.sequoiadb.exception.ReliabilityException ;
import com.sequoiadb.fault.Fault ;
import com.sequoiadb.fault.FaultWrapper ;

public class FaultMakeTask extends Task {

    private FaultWrapper faultInstance ;
    private final int MilliSecondsPerSecond = 1000 ;

    public FaultMakeTask( Fault instance, int maxDuration, int checkTimes ) {
        super( instance.getName(), maxDuration ) ;
        // TODO Auto-generated constructor stub
        faultInstance = new FaultWrapper( instance, checkTimes ) ;
    }

    @SuppressWarnings( "static-access" )
    public void run() {
        Random random = new Random() ;
        try {
            Thread.currentThread().sleep(
                    random.nextInt( super.randomStartMaxDuration
                            * MilliSecondsPerSecond ) ) ;
        } catch ( InterruptedException e ) {
            // TODO Auto-generated catch block
        }
        try{
        faultInstance.make() ;
        faultInstance.checkMakeResult() ;
        }catch(ReliabilityException e){
            
        }
        try {
            Thread.currentThread().sleep(
                    random.nextInt( super.randomStartMaxDuration
                            * MilliSecondsPerSecond ) ) ;
        } catch ( InterruptedException e ) {
            // TODO Auto-generated catch block
        }
        try{
        faultInstance.restore() ;
        faultInstance.checkRestoreResult() ;
        }catch(ReliabilityException e ){
            
        }
        
    }

    public void addDependsTask( OperateTask task ) {
        faultInstance.addDependsTask( task ) ;
    }

    @Override
    public boolean init() {
        // TODO Auto-generated method stub
        return true ;
    }

    @Override
    public boolean fini() {
        // TODO Auto-generated method stub
        return true ;
    }
}
