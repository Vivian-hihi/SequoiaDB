package com.sequoiadb.utils;

import java.util.Random;

/**
 * 字符串的工具类
 */
public class StringUtils {

    private static String base = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@#$%^&*()!";
    private static StringBuffer sb;

    /**
     * 加载类时构造一个公共的大字符串
     */
    static {
        sb = new StringBuffer();
        for ( int i = 0; i < 100; i++ ) {
            sb.append( base );
        }
    }

    /**
     * 获取随机字符串
     * 
     * @param length
     * @return String
     * @Author liuxiaoxuan
     * @Date 2019-05-14
     */
    public static String getRandomString( int length ) {
        int start = new Random().nextInt( length );
        int end = start + length;
        StringBuffer newSB = new StringBuffer();
        newSB.append( sb );
        if(end >= newSB.length()) {  return  newSB.toString().substring( 0, length );  }
        return newSB.toString().substring( start, end );
    }
}
