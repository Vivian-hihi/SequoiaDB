package com.sequoiadb.xml2web;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Properties;

public class Configuration {
    private Properties conf = new Properties();;  
    private FileInputStream input;
    
    public String getValue(String confPath,String key){
    	String value = null;
        try{
        	input = new FileInputStream(confPath);
            conf.load(input);
            input.close();
            if(conf.containsKey(key)){
                value=conf.getProperty(key);
                return value;
            }
        }catch(FileNotFoundException ex){
        	System.err.print("ERROR : configuration file not found." );
            ex.printStackTrace();
        }catch(IOException ex){
        	System.err.print("ERROR : failed to load file." );
            ex.printStackTrace();
        }catch (Exception ex) {
            ex.printStackTrace();
        }
		return value; 
    }
    
    public void clear(){
        conf.clear();
    }
}
