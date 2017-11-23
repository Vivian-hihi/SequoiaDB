package com.sequoiadb.testcommon;

import org.testng.ITestResult;
import org.testng.TestListenerAdapter;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Created by laojingtang on 17-11-23.
 */
public class TimePrinterListener extends TestListenerAdapter {
    // if you want some package use this listener,
    // you should add the package name to packageArray.
    private String[] packageArray = {"com.sequoiadb.lob.randomwrite"};

    @Override
    public void onConfigurationSuccess(ITestResult itr) {
        super.onConfigurationSuccess(itr);
        if (itr.getMethod().isAfterClassConfiguration() && isPrint(itr))
            printEndTime(itr);
    }

    @Override
    public void onConfigurationFailure(ITestResult itr) {
        super.onConfigurationFailure(itr);
        if (itr.getMethod().isAfterClassConfiguration() && isPrint(itr))
            printEndTime(itr);
    }

    @Override
    public void onConfigurationSkip(ITestResult itr) {
        super.onConfigurationSkip(itr);
        if (itr.getMethod().isAfterClassConfiguration() && isPrint(itr))
            printEndTime(itr);
    }

    @Override
    public void beforeConfiguration(ITestResult tr) {
        super.beforeConfiguration(tr);
        if (tr.getMethod().isBeforeClassConfiguration() && isPrint(tr))
            printBeginTime(tr);
    }

    private void printBeginTime(ITestResult tr) {
        System.out.println(getTestMethodName(tr) + " begin: " + getCurTimeStr());
    }

    private void printEndTime(ITestResult tr) {
        System.out.println(getTestMethodName(tr) + " end: " + getCurTimeStr());
    }

    private boolean isPrint(ITestResult tr) {
        String s = tr.getTestClass().getRealClass().getPackage().getName();
        for (String s1 : packageArray) {
            if (s.equals(s1))
                return true;
        }
        return false;
    }

    private String getTestMethodName(ITestResult tr) {
        return tr.getTestClass().getRealClass().getName() + "." + tr.getMethod().getMethodName();
    }

    private String getCurTimeStr() {
        return new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date());
    }
}
