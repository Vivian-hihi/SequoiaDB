package com.sequoiadb.testcommon;

import org.testng.ITestResult;
import org.testng.TestListenerAdapter;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.base.Sequoiadb;

import java.text.SimpleDateFormat;
import java.util.Arrays ;
import java.util.Date;

/**
 * Created by laojingtang on 17-11-23.
 */
public class TimePrinterListener extends TestListenerAdapter {

    private static String[] runGroups = {SdbTestBase.RC, SdbTestBase.RCAUTO,
        SdbTestBase.RCUSERBS, SdbTestBase.RCWAITLOCK, SdbTestBase.RS, SdbTestBase.RU} ;
    
    private boolean isTransCase(String[] groups){
        if ( groups == null || groups.length == 0 ){
            return false ;
        }
        for ( int i = 0; i < runGroups.length; ++i){
            if ( Arrays.asList( groups ).contains( runGroups[i] )){
                return true ;
            }
        }
        
        return false ;
    }
    @Override
    public void onTestStart(ITestResult itr){
        super.onTestStart( itr ) ;
        if ( isTransCase( itr.getMethod().getGroups()) ){
            SdbTestBase.incCaseNum() ;
        }
    }
    
    @Override
    public void onTestSuccess(ITestResult itr){
        super.onTestSuccess( itr ) ;
        if ( isTransCase( itr.getMethod().getGroups()) ){
            SdbTestBase.decCaseNum() ;
        }
    }
    
    @Override
    public void onTestFailure(ITestResult itr){
        super.onTestFailure( itr ) ;
        if ( isTransCase( itr.getMethod().getGroups()) ){
            SdbTestBase.decCaseNum() ;
        }
    }
    
    @Override
    public void onTestSkipped(ITestResult itr){
        super.onTestFailure( itr ) ;
        if ( isTransCase( itr.getMethod().getGroups()) ){
            SdbTestBase.decCaseNum() ;
        }
    }
    
    @Override
    public void onTestFailedButWithinSuccessPercentage(ITestResult itr){
        super.onTestFailedButWithinSuccessPercentage(itr) ;
        if ( isTransCase( itr.getMethod().getGroups()) ){
            SdbTestBase.decCaseNum() ;
        }
    }
    
    
    @Override
    public void onConfigurationSuccess(ITestResult itr) {
        super.onConfigurationSuccess(itr);
        if (itr.getMethod().isAfterClassConfiguration()) {
            printEndTime(itr);
            dbMsgEndTime(itr);
        }
    }

    @Override
    public void onConfigurationFailure(ITestResult itr) {
        super.onConfigurationFailure(itr);
        if (itr.getMethod().isAfterClassConfiguration()) {
            printEndTime(itr);
            dbMsgEndTime(itr);
        }
    }

    @Override
    public void onConfigurationSkip(ITestResult itr) {
        super.onConfigurationSkip(itr);
        if (itr.getMethod().isAfterClassConfiguration()) {
            printEndTime(itr);
            dbMsgEndTime(itr);
        }
    }

    @Override
    public void beforeConfiguration(ITestResult tr) {
        super.beforeConfiguration(tr);
        if (tr.getMethod().isBeforeClassConfiguration()) {
            SdbTestBase.setRunGroup( tr.getMethod().getGroups() ) ;
            printBeginTime(tr);
            dbMsgBeginTime(tr);
        }
    }

    private void printBeginTime(ITestResult tr) {
        System.out.println(getCurTimeStr() + "\tBegin testcase: " + getTestMethodName(tr));
    }

    private void printEndTime(ITestResult tr) {
        System.out.println(getCurTimeStr() + "\tEnd testcase: " + getTestMethodName(tr));
    }

    private String getTestMethodName(ITestResult tr) {
        return tr.getTestClass().getRealClass().getName();
    }

    private String getCurTimeStr() {
        return new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date());
    }
    
    private void dbMsgBeginTime(ITestResult tr) {
        try {
            Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            sdb.msg(getCurTimeStr() + "\tBegin testcase: " + getTestMethodName(tr));
        } catch (BaseException e) {
            e.printStackTrace();
        }
    }
    
    private void dbMsgEndTime(ITestResult tr) {
        try {
            Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            sdb.msg(getCurTimeStr() + "\tEnd testcase: " + getTestMethodName(tr));
        } catch (BaseException e) {
            e.printStackTrace();
        }
    }
}
