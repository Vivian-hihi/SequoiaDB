package com.sequoiadb.testcommon;

import org.testng.ITestResult;
import org.testng.TestListenerAdapter;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Created by laojingtang on 17-11-23.
 */
public class TimePrinterListener extends TestListenerAdapter {
    //the package use this listener
    // if you want some package use this listener,you should add the package name to packageArray.
    private String[] packageArray = {"com.sequoiadb.lob.randomwrite"};

    @Override
    public void onTestStart(ITestResult result) {
        super.onTestStart(result);
        if (isPrint(result))
            printBeginTime(result);
    }

    @Override
    public void onTestFailure(ITestResult tr) {
        super.onTestFailure(tr);
        if (isPrint(tr))
            printEndTime(tr);
    }

    @Override
    public void onTestSkipped(ITestResult tr) {
        super.onTestSkipped(tr);
        if (isPrint(tr))
            printEndTime(tr);
    }

    @Override
    public void onTestSuccess(ITestResult tr) {
        super.onTestSuccess(tr);
        if (isPrint(tr))
            printEndTime(tr);
    }

    @Override
    public void onTestFailedButWithinSuccessPercentage(ITestResult tr) {
        super.onTestFailedButWithinSuccessPercentage(tr);
        if (isPrint(tr))
            printEndTime(tr);
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
