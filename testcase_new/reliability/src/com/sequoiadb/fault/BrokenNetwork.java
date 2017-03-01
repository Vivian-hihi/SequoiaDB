/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:BrokenNetwork.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.fault ;

import java.io.IOException ;

import org.testng.annotations.Test ;

import com.sequoiadb.commlib.SdbTestBase ;
import com.sequoiadb.commlib.Ssh ;
import com.sequoiadb.exception.FaultException ;
import com.sequoiadb.exception.ReliabilityException ;

public class BrokenNetwork extends Fault {
    private String hostName ;
    private String user ;
    private String passwd ;
    private int duration ;
    private int port = 22 ;
    private String remotePath ;
    private Ssh ssh ;
    private long brokenTime ;
    private final String localScriptPath = "./script" ;
    private final String scriptName = "brokenNetwork.sh" ;

    @Test
    public static void test() throws ReliabilityException {
        BrokenNetwork bn = new BrokenNetwork( "192.168.31.31", 15 ) ;
        bn.setPwd( "sequoiadb" ) ;
        bn.setRemotePath( "/tmp" ) ;
        bn.init() ;
        for ( int i = 0; i < 3; i++ )
            System.out.println( bn.ping() ) ;
        try {
            bn.make() ;
        } catch ( ReliabilityException e ) {
            e.printStackTrace() ;
        }
        for ( int i = 0; i < 3; i++ )
            System.out.println( bn.ping() ) ;
        System.out.println( "make:" + bn.checkMakeResult() ) ;

        bn.restore() ;
        System.out.println( "restore:" + bn.checkRestoreResult() ) ;
        bn.fini() ;
    }

    // for debug
    public void setPort( int port ) {
        this.port = port ;
    }

    // for debug
    public void setUser( String user ) {
        this.user = user ;
    }

    // for debug
    public void setPwd( String passwd ) {
        this.passwd = passwd ;
    }

    // for debug
    public void setRemotePath( String remotePath ) {
        this.remotePath = remotePath ;
    }

    /**
     * 
     * @param hostName
     * @param user
     * @param passwd
     * @param remotePath
     * @param duration
     */
    public BrokenNetwork( String hostName, int duration ) {
        super( "brokenNetwork" ) ;
        this.hostName = hostName ;
        this.user = "root" ;
        this.passwd = SdbTestBase.rootPwd ;
        this.duration = duration ;
        this.remotePath = SdbTestBase.workDir ;
        this.port = 22 ;
    }

    public void make() throws FaultException {
        try {
            ssh.execBackground( "nohup " + remotePath + "/" + scriptName + " "
                    + duration + " &" ) ;
            brokenTime = System.currentTimeMillis() ;
        } catch ( ReliabilityException e ) {
            throw new FaultException( e ) ;
        }
    }

    public boolean checkMakeResult() throws FaultException {
        int checkTime = 3 ;
        for ( int i = 0; i < checkTime; i++ ) {
            if ( ping() == false ) {
                return true ;
            }
        }
        return false ;
    }

    public void restore() throws FaultException {
        long diff = System.currentTimeMillis() - brokenTime ;
        if ( diff < duration * 1000 ) {
            try {
                Thread.sleep( duration * 1000 - diff ) ;
            } catch ( InterruptedException e ) {

            }
        }

    }

    public boolean checkRestoreResult() throws FaultException {
        int checkTime = 3 ;
        for ( int i = 0; i < checkTime; i++ ) {
            if ( ping() ) {
                return true ;
            }
        }
        return false ;
    }

    public boolean ping() throws FaultException {
        String os = System.getProperties().getProperty( "os.name" ) ;
        String cmd ;
        if ( os.startsWith( "win" ) || os.startsWith( "Win" ) ) {
            cmd = "ping " + hostName + " -n 1 -w 1" ;
        } else {
            cmd = "ping " + hostName + " -c 1 -w 1" ;
        }
        Runtime rt = Runtime.getRuntime() ;
        try {
            Process pr = rt.exec( cmd ) ;
            pr.waitFor() ;
            int exitcode = pr.exitValue() ;
            pr.destroy() ;
            if ( exitcode == 0 ) {
                return true ;
            } else {
                return false ;
            }
        } catch ( InterruptedException | IOException e ) {
            throw new FaultException( e ) ;
        }
    }

    @Override
    public boolean init() throws FaultException {
        try {
            if ( ssh == null ) {
                ssh = new Ssh( hostName, user, passwd, port ) ;
            }
            ssh.scpTo( localScriptPath + "/" + scriptName, remotePath ) ;
            ssh.exec( "chmod 777 " + remotePath + "/" + scriptName ) ;
        } catch ( ReliabilityException e ) {
            throw new FaultException( e ) ;
        }
        return true ;
    }

    @Override
    public boolean fini() throws FaultException {
        if ( ssh != null ) {
            try {
                ssh.exec( "rm -rf " + remotePath + "/" + scriptName ) ;
            } catch ( ReliabilityException e ) {
                throw new FaultException( e ) ;

            } finally {
                ssh.close() ;
            }
        }
        return true ;
    }

}
