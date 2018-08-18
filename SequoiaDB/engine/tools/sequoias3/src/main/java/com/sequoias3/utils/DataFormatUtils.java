package com.sequoias3.utils;

import java.text.SimpleDateFormat;
import java.util.Date;

public class DataFormatUtils {
    public static final String DATA_PATTERN = "yyyy-MM-dd'T'HH:mm:ss.SSS'Z'";

    public static String formatDate(long time){
        SimpleDateFormat sdf = new SimpleDateFormat(DATA_PATTERN);
        return sdf.format(new Date(time));
    }
}
