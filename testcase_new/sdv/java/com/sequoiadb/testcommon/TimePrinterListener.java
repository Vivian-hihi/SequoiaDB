package com.sequoiadb.testcommon;

import org.testng.ITestResult;
import org.testng.TestListenerAdapter;

import com.sequoiadb.base.Sequoiadb;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Created by laojingtang on 17-11-23.
 */
public class TimePrinterListener extends TestListenerAdapter {

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
            printBeginTime(tr);
            dbMsgBeginTime(tr);
        }
    }

    private void printBeginTime(ITestResult tr) {
        System.out.println(getCurTimeStr() + "\tbegin: " + getTestMethodName(tr));
    }

    private void printEndTime(ITestResult tr) {
        System.out.println(getCurTimeStr() + "\tend  : " + getTestMethodName(tr));
    }

    private String getTestMethodName(ITestResult tr) {
        return tr.getTestClass().getRealClass().getName();
    }

    private String getCurTimeStr() {
        return new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date());
    }
    
    private void dbMsgBeginTime(ITestResult tr) {
        Sequoiadb sdb = new Sequoiadb("localhost:11810", "", "");
        sdb.msg(getCurTimeStr() + "\tbegin: " + getTestMethodName(tr));
    }
    
    private void dbMsgEndTime(ITestResult tr) {
        Sequoiadb sdb = new Sequoiadb("localhost:11810", "", "");
        sdb.msg(getCurTimeStr() + "\tend  : " + getTestMethodName(tr));
    }
}
