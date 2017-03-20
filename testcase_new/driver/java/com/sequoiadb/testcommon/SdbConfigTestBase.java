package com.sequoiadb.testcommon;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.util.Arrays;
import java.util.Properties;

import org.testng.Assert;
import org.testng.annotations.AfterTest;
import org.testng.annotations.BeforeTest;
import org.testng.annotations.Parameters;

public class SdbConfigTestBase extends SdbTestBase{
    protected static String toolFullName;
        
    @Parameters({"CONFTOOL"})
    
    @BeforeTest( alwaysRun = true )
    public void initTest( String CONFTOOL ){
        toolFullName = CONFTOOL;
        
        try{
            String[] cmd = getConfCmd( "before" );
            System.out.println( "exec cmd: " + Arrays.toString( cmd ) );
            Process process = Runtime.getRuntime().exec( cmd );
         
            BufferedReader input = new BufferedReader( new InputStreamReader( process.getInputStream() ) );
            String line = "";
            while( (line = input.readLine()) != null ){
                System.out.println(line);
            }
         
            int exitValue = process.waitFor();
            if( 0 != exitValue ){
                Assert.fail( "fail to change node configure beforetest, return code=" + exitValue );
            }      
        }
        catch( InterruptedException | IOException e ){
            e.printStackTrace();
            Assert.fail( "fail to change node configure beforetest" );
        }
    }
  
    @AfterTest( alwaysRun = true )
    public void finiTest(){
        try{
            String[] cmd = getConfCmd( "after" );
            System.out.println( "exec cmd: " + Arrays.toString( cmd ) );
            Process process = Runtime.getRuntime().exec( cmd );
         
            BufferedReader input = new BufferedReader( new InputStreamReader( process.getInputStream() ) );
            String line = "";
            while( (line = input.readLine()) != null ){
                System.out.println(line);
            }
         
            int exitValue = process.waitFor();
            if( 0 != exitValue ){
                Assert.fail( "fail to change node configure aftertest, return code=" + exitValue );
            }      
        }
        catch( InterruptedException | IOException e ){
            e.printStackTrace();
            Assert.fail( "fail to change node configure aftertest" );
        }
    }
   
    private String[] getConfCmd( String mode ){
        String[] cmd = new String[5];
        try{           
            String confFullName = "";
            if( mode == "before" ){
                confFullName = this.getClass().getResource("") + "/node.conf";
                confFullName = confFullName.substring( 5 );
            }
            else{
                confFullName = System.getProperty( "user.dir" ) + "/node.conf.ini";
            }
            
            Properties prop = new Properties();
            InputStream in = new FileInputStream( new File("/etc/default/sequoiadb") );
            prop.load( in );
            String installPath = prop.getProperty( "INSTALL_DIR" );
            String sdbFullName = installPath + "/bin/sdb";
       
            cmd[0] = sdbFullName;
            cmd[1] = "-f";
            cmd[2] = confFullName + "," + toolFullName;
            cmd[3] = "-e";
            cmd[4] = "var hostname='" + hostName + "';" +
                     "var svcname=" + serviceName + ";" +
                     "var mode='" + mode + "'";
           
        }catch( IOException e ){
            e.printStackTrace();
            Assert.fail("get configure command failed");
        }       
        return cmd ;
    }
}
