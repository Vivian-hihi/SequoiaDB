package com.sequoias3.utils;

import java.util.Random;

public class IDUtils {

    public static String getAccessKeyID(){
        Random random = new Random();
        StringBuffer accessKey = new StringBuffer();

        for (int index=0; index < 20; index++){
            int number = random.nextInt(3);
            long result = 0;
            switch (number){
                case 0:
                case 1:
                    result = Math.round(Math.random()*25+65);
                    accessKey.append(String.valueOf((char)result));
                    break;
                case 2:
                    accessKey.append(String.valueOf(new Random().nextInt(10)));
                    break;
            }
        }

        return accessKey.toString();
    }

    public static String getSecretKey(){
        Random random = new Random();
        StringBuffer secretAccessKey = new StringBuffer();

        for (int index=0; index < 40; index++){
            int number = random.nextInt(4);
            long result = 0;
            switch (number){
                case 0:
                    result = Math.round(Math.random()*25+65);
                    secretAccessKey.append(String.valueOf((char)result));
                    break;
                case 1:
                case 2:
                    result = Math.round(Math.random()*25+97);
                    secretAccessKey.append(String.valueOf((char)result));
                    break;
                case 3:
                    secretAccessKey.append(String.valueOf(new Random().nextInt(10)));
                    break;
            }
        }
        return secretAccessKey.toString();
    }
}
