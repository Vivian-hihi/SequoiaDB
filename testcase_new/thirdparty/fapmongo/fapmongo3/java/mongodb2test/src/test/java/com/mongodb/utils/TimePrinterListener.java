package com.mongodb.utils;

import org.apache.log4j.Logger;
import org.testng.ITestResult;
import org.testng.TestListenerAdapter;

/**
 * Created by laojingtang on 17-11-23.
 */
public class TimePrinterListener extends TestListenerAdapter {
    private static final Logger logger = Logger
            .getLogger( TimePrinterListener.class );

    @Override
    public void onConfigurationSuccess( ITestResult itr ) {
        super.onConfigurationSuccess( itr );
        if ( itr.getName().equals( "springTestContextAfterTestClass" ) ) {
            printEndTime( itr );
        }
    }

    @Override
    public void onConfigurationFailure( ITestResult itr ) {
        super.onConfigurationFailure( itr );
        if ( itr.getMethod().isAfterClassConfiguration() ) {
            printEndTime( itr );
        }
    }

    @Override
    public void onConfigurationSkip( ITestResult itr ) {
        super.onConfigurationSkip( itr );
        if ( itr.getMethod().isAfterClassConfiguration() ) {
            printEndTime( itr );
        }
    }

    @Override
    public void beforeConfiguration( ITestResult tr ) {
        super.beforeConfiguration( tr );
        if ( tr.getName().equals( "springTestContextBeforeTestClass" ) ) {
            printBeginTime( tr );
        }
    }

    private void printBeginTime( ITestResult tr ) {
        logger.info( "\tBegin testcase: " + getTestMethodName( tr ) );
    }

    private void printEndTime( ITestResult tr ) {
        logger.info( "\tEnd testcase: " + getTestMethodName( tr ) );
    }

    private String getTestMethodName( ITestResult tr ) {
        return tr.getTestClass().getRealClass().getName();
    }
}
